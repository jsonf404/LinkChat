#!/bin/sh
set -eu

template="/config/${LINKCHAT_SERVICE}.ini.template"
if [ ! -r "$template" ]; then
    echo "Missing service configuration template: $template" >&2
    exit 1
fi

envsubst < "$template" > /app/config.ini
exec "/app/${LINKCHAT_SERVICE}"
