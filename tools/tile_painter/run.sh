#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cleaned=""
if [ -n "${LD_LIBRARY_PATH:-}" ]; then
    IFS=: read -ra parts <<< "$LD_LIBRARY_PATH"
    for p in "${parts[@]}"; do
        case "$p" in
            /snap/*) ;;
            *) [ -n "$cleaned" ] && cleaned="$cleaned:$p" || cleaned="$p" ;;
        esac
    done
fi
export LD_LIBRARY_PATH="$cleaned"

exec "$SCRIPT_DIR/../.venv/bin/python" -m streamlit run "$SCRIPT_DIR/app.py" "$@"
