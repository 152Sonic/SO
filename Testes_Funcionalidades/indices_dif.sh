./logidxreader | grep co| cut --delimiter=',' -f 3 | wc -l
./logidxreader | grep co| cut --delimiter=',' -f 3 | uniq | wc -l
