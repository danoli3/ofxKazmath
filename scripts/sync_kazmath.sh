#!/bin/sh
# Sync the bundled kazmath C library with upstream Kazade/kazmath.
#
# Layout mapping (upstream keeps headers and sources side by side,
# we keep the openFrameworks-style split):
#   upstream kazmath/*.h      -> libs/kazmath/include/kazmath/*.h
#   upstream kazmath/*.c      -> libs/kazmath/src/*.c
#   upstream kazmath/GL/*.[ch] -> libs/kazmath/include|src/kazmath/GL/
#
# Usage:
#   scripts/sync_kazmath.sh [--check] [--ref <git-ref>] [--repo <url>] [--force]
#
#   --check        Compare only; report SAME/DIFFER/NEW/REMOVED per file and
#                  exit 1 when the bundled copy has drifted. Makes no changes.
#   --ref <ref>    Upstream branch, tag or SHA to sync to (default: master).
#   --repo <url>   Upstream git URL (default: https://github.com/Kazade/kazmath.git).
#   --force        Sync even when libs/kazmath has uncommitted changes.
#
# On a real sync the upstream commit SHA is recorded in
# libs/kazmath/.upstream_commit.

set -u

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(dirname -- "$SCRIPT_DIR")
LIB="$ROOT/libs/kazmath"
UPSTREAM_URL="https://github.com/Kazade/kazmath.git"
REF="master"
MODE="sync"
FORCE=0

while [ $# -gt 0 ]; do
    case "$1" in
        --check) MODE="check"; shift ;;
        --ref) REF="$2"; shift 2 ;;
        --repo) UPSTREAM_URL="$2"; shift 2 ;;
        --force) FORCE=1; shift ;;
        -h|--help)
            sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'
            exit 0 ;;
        *) echo "error: unknown argument: $1" >&2; exit 2 ;;
    esac
done

if [ "$MODE" = "sync" ] && [ "$FORCE" -eq 0 ]; then
    if [ -n "$(git -C "$ROOT" status --porcelain -- libs/kazmath 2>/dev/null)" ]; then
        echo "error: libs/kazmath has uncommitted changes; commit them or re-run with --force." >&2
        exit 1
    fi
fi

TMPDIR_BASE="${TMPDIR:-/tmp}"
WORK=$(mktemp -d "$TMPDIR_BASE/kazmath-sync.XXXXXX") || exit 1
trap 'rm -rf "$WORK"' EXIT INT TERM

echo "Cloning $UPSTREAM_URL ($REF)..."
git clone -q "$UPSTREAM_URL" "$WORK/upstream" 2>&1 | grep -v "^$" || true
git -C "$WORK/upstream" checkout -q "$REF" -- 2>/dev/null || {
    echo "error: cannot check out ref '$REF'." >&2
    exit 1
}
SHA=$(git -C "$WORK/upstream" rev-parse HEAD)
UP="$WORK/upstream/kazmath"
echo "Upstream commit: $SHA"

drift=0
synced=0
report() {
    # $1 = status, $2 = relative path under libs/kazmath
    printf '%-8s %s\n' "$1" "$2"
    if [ "$1" != "SAME" ]; then
        drift=$((drift + 1))
    fi
}

# $1 = upstream subdir ("" or "GL"), $2 = extension, $3 = dest subdir
sync_group() {
    usub="$1"; ext="$2"; dest="$3"
    if [ -n "$usub" ]; then
        srcdir="$UP/$usub"; rel="$usub"
    else
        srcdir="$UP"; rel=""
    fi
    destdir="$LIB/$dest"
    [ -n "$rel" ] && destdir="$destdir/$rel"

    for src in "$srcdir"/*."$ext"; do
        [ -e "$src" ] || continue
        name=$(basename "$src")
        dst="$destdir/$name"
        if [ -n "$rel" ]; then
            disp="$dest/$rel/$name"
        else
            disp="$dest/$name"
        fi
        if [ ! -f "$dst" ]; then
            report "NEW" "$disp"
            if [ "$MODE" = "sync" ]; then
                mkdir -p "$destdir"
                cp "$src" "$dst"
                synced=$((synced + 1))
            fi
        elif cmp -s "$src" "$dst"; then
            report "SAME" "$disp"
        else
            report "DIFFER" "$disp"
            if [ "$MODE" = "sync" ]; then
                cp "$src" "$dst"
                synced=$((synced + 1))
            fi
        fi
    done

    # Bundled files with no upstream counterpart.
    if [ -d "$destdir" ]; then
        for dst in "$destdir"/*."$ext"; do
            [ -e "$dst" ] || continue
            name=$(basename "$dst")
            if [ ! -f "$srcdir/$name" ]; then
                if [ -n "$rel" ]; then
                    disp="$dest/$rel/$name"
                else
                    disp="$dest/$name"
                fi
                report "REMOVED" "$disp"
                if [ "$MODE" = "sync" ]; then
                    rm "$dst"
                    synced=$((synced + 1))
                fi
            fi
        done
    fi
}

sync_group ""  h "include/kazmath"
sync_group ""  c "src"
sync_group "GL" h "include/kazmath"
sync_group "GL" c "src"

if [ "$MODE" = "check" ]; then
    if [ "$drift" -eq 0 ]; then
        echo "Bundled kazmath is up to date with $SHA."
        exit 0
    else
        echo "$drift file(s) differ from upstream $SHA."
        exit 1
    fi
fi

printf '%s\n' "$SHA" > "$LIB/.upstream_commit"
echo "Synced $synced file(s) to $SHA (recorded in libs/kazmath/.upstream_commit)."
