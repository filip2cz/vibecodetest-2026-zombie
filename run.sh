#!/usr/bin/env bash
nix-shell -p raylib gcc gnumake pkg-config --run "make && ./game"
