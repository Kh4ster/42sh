function test()
{
    echo function
    if false;then
        echo bad;
    elif false;then
        echo bad
    else
        echo good;
    fi
}

test a b c; echo ok; echo 1 && echo 3
