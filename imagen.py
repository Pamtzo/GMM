from PIL import Image
import os
data=open("dataset256.txt",'a')
data.write("RED,GREEN,BLUE,NUMERO,ESTILO\n")
data.close()

data=open("dataset512.txt",'a')
data.write("RED,GREEN,BLUE,NUMERO,ESTILO\n")
data.close()

def datagen(directory, etiqueta,size):
    paso=0
    for name in os.listdir(directory):
        print("Paso #{} de {}, {}".format(paso, len(os.listdir(directory)), name))
        im = Image.open(directory+'/'+name) # Can be many different formats.
        im=im.resize((size,size),Image.NEAREST)
        im.save(etiqueta+"/"+str(size)+"/"+name)
        pix = im.load()
        print (im.size)  # Get the width and hight of the image for iterating over

        if type(pix[0,0]) is tuple:
            data=open("dataset"+str(size)+".txt",'a')
            colores=[]
            RED=0
            GREEN=0
            BLUE=0

            for x in range(im.size[0]):
                for y in range(im.size[1]):
                    if pix[x,y] not in colores:
                        colores.append(pix[x,y])
                    RED+=pix[x,y][0]
                    GREEN+=pix[x,y][1]
                    BLUE+=pix[x,y][2]
            
            paso+=1
            data.write(str(RED/40000)+","+str(GREEN/40000)+","+str(BLUE/40000)+","+str(len(colores))+","+etiqueta+'\n')
            data.close()

datagen("Dataset","americano",512)
datagen("Asiatico","asiatico",512)