#!/bin/bash

mkdir -p ./dist

source_dir="../dist"
install_path="$HOME/bin/kovacs"

mkdir -p "$install_path"

cp "$source_dir"/* "$install_path"

if [[ ":$PATH:" != *":$install_path:"* ]]; then
    echo "export PATH=\$PATH:$install_path" >> "$HOME/.bashrc"
    source "$HOME/.bashrc"
    echo "The directory '$install_path' has been added to the PATH."
else
    echo "The directory '$install_path' is already in the PATH."
fi