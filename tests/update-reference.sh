#!/usr/bin/env sh

set -euo pipefail

cd "$(dirname "$0")/.." || exit 1

ref_url='https://github.com/material-foundation/material-color-utilities'
ref_lock='tests/reference.lock'
ref_out='.reference'
ref_out_sha="$ref_out/SHA"

if [ -f "$ref_lock" ] && [ "${1:-}" != '--pull' ]; then
    ref_sha="$(cat "$ref_lock")"
    echo "Using ${ref_lock} (${ref_sha})"
else
    [ -e "$ref_lock" ] && rm "$ref_lock"
    ref_sha="$(git ls-remote "$ref_url" HEAD | cut -f1)"
    echo "$ref_sha" > "$ref_lock"
    echo "Updated ${ref_lock} to ${ref_sha}"
fi

if [ -d "$ref_out" ] && [ "$(cat "$ref_out_sha")" != "$ref_sha" ]; then
    rm -rf "$ref_out"
    echo "Removed ${ref_out}"
fi

if [ ! -d "$ref_out" ]; then
    mkdir -p "$ref_out"
    curl -L "${ref_url}/archive/${ref_sha}.tar.gz" | tar -xz --strip-components=1 -C "$ref_out"
    echo "$ref_sha" > "$ref_out_sha"
    echo "Updated ${ref_out}/ to ${ref_sha}"
else
    echo "${ref_out}/ is already up to date"
fi
