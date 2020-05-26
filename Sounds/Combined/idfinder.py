metafile = open('Files.txt', 'r')
filenames = metafile.readlines()
metafile.close()

outputfile = open("ClipIDs.txt", 'w')
for i in range(len(filenames)):
    outputfile.write("SOUND_" + filenames[i].upper()[:filenames[i].find('.')] + " = ( 1 << " + str(i) + " ),\n")
outputfile.close()
