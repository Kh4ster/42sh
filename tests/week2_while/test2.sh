touch file
var1=titi
var2=end
while cat file; do
    echo $var1
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
    echo $var2
done >> file
cat file
rm file

