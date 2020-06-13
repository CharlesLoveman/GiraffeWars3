import wave
import contextlib
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

    for name in filenames:
        name = name.rstrip()
        outputf.write("{");
        with contextlib.closing(wave.open(name, 'r')) as f:
            frames = f.getnframes()
            rate = f.getframerate()
            duration = int((frames / float(rate)) * 60)
            outputf.write(str(duration) + ",")
        outputf.write("};");

    outputf.close()

if __name__ == "__main__":
    main(sys.argv[1:])
