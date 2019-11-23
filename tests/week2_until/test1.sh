var=toto
until var=papa; do
    if true; then
        echo toto
    else echo papa
    fi
    var=papa
done
