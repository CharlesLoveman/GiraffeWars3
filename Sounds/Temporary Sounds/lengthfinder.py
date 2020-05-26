import wave
import contextlib

metafile = open('Files.txt', 'r')
filenames = metafile.readlines()
metafile.close()


for name in filenames:
    name = name.rstrip()
    with contextlib.closing(wave.open(name, 'r')) as f:
        frames = f.getnframes()
        rate = f.getframerate()
        duration = int((frames / float(rate)) * 60)
        print("#define TESTBANK_" + name.upper()[:name.find('.')] + "_LENGTH", duration)


print()

for i in range(len(filenames)):
    print("SOUND_" + filenames[i].rstrip().upper()[:filenames[i].find('.')] + " = ( 1 <<", i, "),")

