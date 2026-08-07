# SPDX-License-Identifier: CC0-1.0

import pathlib
from hashlib import md5

import streamlit as st
import streamlit.components.v1 as components
import streamlit.elements.image as st_image

_component = components.declare_component(
    "tile_canvas",
    path=str(pathlib.Path(__file__).parent / "frontend"),
)


def tile_canvas(background, grid, cols, rows, tile, paint_value, rev=0,
                key=None):
    width = cols * tile
    height = rows * tile
    img = background.convert("RGB").resize((width, height))
    url = st_image.image_to_url(
        img, width, True, "RGB", "PNG",
        "tilecanvas-bg-%s-%s" % (md5(img.tobytes()).hexdigest(), key),
    )
    url = st._config.get_option("server.baseUrlPath") + url
    return _component(
        backgroundUrl=url,
        grid=[int(v) for v in grid.flat],
        cols=int(cols),
        rows=int(rows),
        tile=int(tile),
        paintValue=int(paint_value),
        rev=int(rev),
        key=key,
        default=None,
    )
