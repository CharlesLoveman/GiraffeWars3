XWBTool.exe -o AttackBank.xwb -h AttackBankHeader.h -l -y -flist AttackFiles.txt
Python lengthfinder.py -i AttackFiles.txt -o AttackLengths.txt
Python idfinder.py -i AttackFiles.txt -o AttackIDs.txt
XWBTool.exe -o MoveBank.xwb -h MoveBankHeader.h -l -y -flist MoveFiles.txt
Python lengthfinder.py -i MoveFiles.txt -o MoveLengths.txt
Python idfinder.py -i MoveFiles.txt -o MoveIDs.txt