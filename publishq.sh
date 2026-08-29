#!/bin/bash

APP_NAME=$1
SRC_FILE=$2
REPO_URL=$3

if [ -z "$APP_NAME" ] || [ -z "$SRC_FILE" ] || [ -z "$REPO_URL" ]; then
    exit 1
fi

APP_DIR="published/$APP_NAME"

if ! grep -q "^published/" .gitignore 2>/dev/null; then
    echo "published/" >> .gitignore
fi

mkdir -p "$APP_DIR"
cp "$SRC_FILE" "$APP_DIR/$APP_NAME.c"
cp levo.c "$APP_DIR/"
cp levo.h "$APP_DIR/"

cd "$APP_DIR" || exit

git init
git branch -M main
git remote add origin "$REPO_URL"
git add .
git commit -m "Initial commit"
git push -u origin main
