
mkdir -p sandbox
cd sandbox

rm -rf temp

mkdir temp
ln -s file.txt temp/symlink
echo "123" > temp/file.txt
link temp/file.txt temp/hardlink

mkdir -p temp.backup

cd ../..
make
cd -

../../backup temp temp.backup

