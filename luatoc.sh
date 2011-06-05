if [ $# -ne 3 ]; then
    echo "Usage: `basename $0` <input file> <output file> <variable name>"
    exit 1
fi

{
    echo "extern const char $3[] = {"
    od --format=x1 --output-duplicates $1 \
        | cut --delimiter=' ' --fields=2- --only-delimited \
        | sed -e 's/^/    /' -e 's/\>/,/g' -e 's/\</0x/g'
    echo "};"
    echo
    echo "extern const unsigned int $3_size = sizeof $3;"
} > $2
