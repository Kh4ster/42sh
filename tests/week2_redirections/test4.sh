for a in a b c; do
    echo file 1 >> file1 >> file3
    echo file 2
done > file2
cat file3
cat file1
cat file2
rm file1 file2 file3
