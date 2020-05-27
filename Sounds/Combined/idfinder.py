import sys
import getopt

def main(argv):
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv, "hi:o:", ["ifile=","ofile="])
    except getopt.GetoptError:
        print("lengthfinder.py -i <inputfile> -o <outputfile>")
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
            
    metafile = open(inputfile, 'r')
    filenames = metafile.readlines()
    metafile.close()

    outputf = open(outputfile, 'w')
    for i in range(len(filenames)):
        outputf.write("SOUND_" + filenames[i].upper()[:filenames[i].find('.')] + " = ( 1 << " + str(i) + " ),\n")
    outputf.close()

if __name__ == "__main__":
    main(sys.argv[1:])
