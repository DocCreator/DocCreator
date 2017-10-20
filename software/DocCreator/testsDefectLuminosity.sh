#!/bin/sh

heure=$(date +%Hh%M)
jour=$(date +%Y-%m-%d)
mkdir "${HOME}/testDegradations/Luminosite/images/$heure\_$jour"

fileHTML=${HOME}/testDegradations/Luminosite/$heure\_$jour.html
imageFile="${HOME}/testDegradations/Luminosite/images/$heure\_$jour"

if [ -z $1 ]; then
    dist=200
else
    dist=$1
fi
if [ -z $2 ]; then
    A=0.0005
else
    A=$2
fi
if [ -z $3 ]; then
    name="name"
else
    name=$3
fi
if [ -z $4 ]; then
    imageSrc="${HOME}/LABRI/sDEfauts/CESR_287_0011.jpg"
else
    imageSrc=$4
fi

#Ecriture de l'entete
echo "<!doctype html>  \n\n <head>\n <meta charset=\"utf-8\">
 <title> Test du $jour à $heure</title>\n <script type=\"text/javascript\" src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.4.2/jquery.min.js\"></script> \n <script type=\"text/javascript\" src=\"zoomer.js\"></script> \n <script type=\"text/javascript\">
jQuery(document).ready(function($){
 \$('#original').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#1').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#2').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#3').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#4').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#5').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#6').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#7').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#8').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
})
</script>\n  </head>\n\n <body>\n
 <h1>Test du $jour à $heure de la luminosité aved D=$dist et A=$A</h1>" >> $fileHTML


#Affichage de l'original

cp $imageSrc "${HOME}/testDegradations/Luminosite/images/$heure\_$jour"
imageSrc="${HOME}/testDegradations/Luminosite/images/$heure\_$jour/CESR_287_0011.jpg"

 echo "<li><p>Original :</p>\n <img id=\"original\" src=\"$imageSrc\" style=\"width:250px; height:400px\"></li>\n </div>" >> $fileHTML

#Affichage des différentes fonctions
for fun in 1 2 3 4 5 6 7 8; do
    ${HOME}/doccreator/build/software/AncientDocumentEditor/testLuminosity $fun $dist $A $name $imageSrc "$imageFile/$fun.jpg"

 echo "<li><p>Fonction $fun :</p>\n <img id=\"$fun\" src=\"$imageFile/$fun.jpg\" style=\"width:250px; height:400px\"></li>\n </div>" >> $fileHTML
done 

#Ecriture fin de fichier html
echo "\n </body>\n\n</html>" >> $fileHTML
