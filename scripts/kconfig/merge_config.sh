#!/usr/bin/env bash

MODULE_NAME="MERGE_CONFIG"
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 base_config.config fragment.config"
    exit 1
fi

BASE_CONF=$1
MERGE_CONF=$2
OUTPUT_CONF=".config"

echo "<6>[  ${MODULE_NAME}  ] Blending target KConfig maps into unified active layouts..."

cp "$BASE_CONF" "$OUTPUT_CONF"

while idx= read -r line; do
    if [[ ! -z "$line" && "$line" != "#"* ]]; then
        key=$(echo "$line" | cut -d'=' -f1)
        # Delete existing line if present inside base blueprint
        sed -i "/^$key=/d" "$OUTPUT_CONF"
        # Append updated override token configuration definition
        echo "$line" >> "$OUTPUT_CONF"
    fi
done < "$MERGE_CONF"

echo "<6>[  ${MODULE_NAME}  ] Configuration matrices successfully integrated into ${OUTPUT_CONF}"