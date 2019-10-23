#!/bin/sh
# FILE USED BY nicolas.blin TO MAKE add, commit, push easier

git add .
git commit -m "$1"
git push