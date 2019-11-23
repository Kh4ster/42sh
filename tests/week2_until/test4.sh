until cat file; do
    echo yes
    if true; then
        echo yes!
    fi
    touch file
done
echo toto
rm file
