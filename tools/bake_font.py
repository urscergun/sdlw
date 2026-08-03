#!/usr/bin/env python3
"""Offline font-baking pipeline for sdlw.

Converts a TrueType/OpenType font into a bitmap-font atlas that sdlw renders
at runtime using only SDL:

    <name>.bmp   24-bit anti-aliased glyph atlas (white glyphs on black;
                 the gray level IS the coverage/alpha). Loads via SDL_LoadBMP.
    <name>.fnt   AngelCode BMFont text descriptor: per-glyph atlas rect,
                 offsets, and advance, plus line metrics.

3rd-party tool in the pipeline: Pillow (which wraps FreeType) does the TTF
rasterization. Everything the game links against is still just SDL.

Usage:
    python bake_font.py FONT.ttf --size 14 --out assets/dejavusans_14
    python bake_font.py FONT.ttf --size 14 --out out/font --first 32 --last 126
"""

import argparse
import os
from PIL import Image, ImageFont, ImageDraw


def glyph_coverage(font, ch):
    """Return (coverage_image_L, width, height, xoffset, yoffset) for `ch`.

    coverage_image_L is an 8-bit 'L' image of the inked box (None if blank).
    xoffset/yoffset are relative to the line box (top-left, baseline at ascent).
    """
    bbox = font.getbbox(ch)  # (l, t, r, b) with (0,0) = left / ascender-top
    if bbox is None:
        return None, 0, 0, 0, 0
    l, t, r, b = bbox
    w, h = r - l, b - t
    if w <= 0 or h <= 0:
        return None, 0, 0, 0, 0
    # Render the glyph and crop to its inked box to get tight AA coverage.
    canvas = Image.new("L", (r + 2, b + 2), 0)
    ImageDraw.Draw(canvas).text((0, 0), ch, fill=255, font=font)
    cov = canvas.crop((l, t, r, b))
    return cov, w, h, l, t


def shelf_pack(boxes, atlas_w, pad):
    """Simple shelf packer. boxes: list of (key, w, h). Returns placements and
    the total atlas height. Boxes should be pre-sorted by height (desc)."""
    x = pad
    y = pad
    shelf_h = 0
    placements = {}
    for key, w, h in boxes:
        if x + w + pad > atlas_w:      # wrap to next shelf
            x = pad
            y += shelf_h + pad
            shelf_h = 0
        placements[key] = (x, y)
        x += w + pad
        shelf_h = max(shelf_h, h)
    return placements, y + shelf_h + pad


def bake(font_path, size, out_base, first, last, atlas_w, pad):
    font = ImageFont.truetype(font_path, size)
    ascent, descent = font.getmetrics()
    line_height = ascent + descent

    codepoints = list(range(first, last + 1))

    # Rasterize every glyph first, then pack.
    glyphs = {}  # cp -> dict(cov,w,h,xoff,yoff,adv)
    for cp in codepoints:
        ch = chr(cp)
        cov, w, h, xoff, yoff = glyph_coverage(font, ch)
        adv = round(font.getlength(ch))
        glyphs[cp] = dict(cov=cov, w=w, h=h, xoff=xoff, yoff=yoff, adv=adv)

    inked = [(cp, g["w"], g["h"]) for cp, g in glyphs.items() if g["cov"] is not None]
    inked.sort(key=lambda t: t[2], reverse=True)  # tallest first
    placements, atlas_h = shelf_pack(inked, atlas_w, pad)

    # Round atlas height up to a power-of-two-ish multiple of 4 (texture-friendly).
    atlas_h = (atlas_h + 3) & ~3

    # Compose the atlas: white glyphs on black; gray level == coverage.
    atlas = Image.new("RGB", (atlas_w, atlas_h), (0, 0, 0))
    for cp, (x, y) in placements.items():
        g = glyphs[cp]
        atlas.paste((255, 255, 255), (x, y), mask=g["cov"])

    face = os.path.splitext(os.path.basename(font_path))[0]
    bmp_path = out_base + ".bmp"
    fnt_path = out_base + ".fnt"
    os.makedirs(os.path.dirname(os.path.abspath(out_base)), exist_ok=True)
    atlas.save(bmp_path)  # 24-bit BMP

    # Write the BMFont text descriptor.
    tex_name = os.path.basename(bmp_path)
    lines = []
    lines.append(
        f'info face="{face}" size={size} bold=0 italic=0 charset="" unicode=1 '
        f'stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing={pad},{pad}'
    )
    lines.append(
        f"common lineHeight={line_height} base={ascent} scaleW={atlas_w} "
        f"scaleH={atlas_h} pages=1 packed=0"
    )
    lines.append(f'page id=0 file="{tex_name}"')
    lines.append(f"chars count={len(codepoints)}")
    for cp in codepoints:
        g = glyphs[cp]
        if g["cov"] is not None:
            x, y = placements[cp]
            w, h, xo, yo = g["w"], g["h"], g["xoff"], g["yoff"]
        else:
            x = y = w = h = xo = yo = 0
        lines.append(
            f"char id={cp} x={x} y={y} width={w} height={h} "
            f"xoffset={xo} yoffset={yo} xadvance={g['adv']} page=0 chnl=15"
        )
    with open(fnt_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    print(f"atlas : {bmp_path}  ({atlas_w}x{atlas_h})")
    print(f"desc  : {fnt_path}  ({len(codepoints)} glyphs, lineHeight={line_height}, base={ascent})")
    return bmp_path, fnt_path


def main():
    ap = argparse.ArgumentParser(description="Bake a TTF into a bitmap-font atlas (.bmp + .fnt).")
    ap.add_argument("font", help="path to .ttf/.otf")
    ap.add_argument("--size", type=int, required=True, help="em pixel size")
    ap.add_argument("--out", required=True, help="output base path (no extension)")
    ap.add_argument("--first", type=int, default=32, help="first codepoint (default 32)")
    ap.add_argument("--last", type=int, default=126, help="last codepoint (default 126)")
    ap.add_argument("--atlas-width", type=int, default=256, help="atlas width in px")
    ap.add_argument("--pad", type=int, default=1, help="padding between glyphs")
    a = ap.parse_args()
    bake(a.font, a.size, a.out, a.first, a.last, a.atlas_width, a.pad)


if __name__ == "__main__":
    main()
