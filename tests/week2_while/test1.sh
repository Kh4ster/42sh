touch file
while cat file; do
    echo toto
    if true; then
        while cat file; do
            echo tata
            while cat file; do
                rm file
            done
        done
    else
        rm file
    fi
    echo end
done

