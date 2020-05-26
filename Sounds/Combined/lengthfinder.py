import wave
import contextlib

metafile = open('Files.txt', 'r')
filenames = metafile.readlines()
metafile.close()


outputfile = open("ClipLengths.txt", 'w')

for name in filenames:
    name = name.rstrip()
    with contextlib.closing(wave.open(name, 'r')) as f:
        frames = f.getnframes()
        rate = f.getframerate()
        duration = int((frames / float(rate)) * 60)
        outputfile.write("#define TESTBANK_" + name.upper()[:name.find('.')] + "_LENGTH " + str(duration) + "\n")

outputfile.close()
