#!/bin/bash

convert forest.png -fx "(r*255 == 211 && g*255 == 207 && b*255 == 178) ? 1 : 0" -resize 12.5% colmap.png

