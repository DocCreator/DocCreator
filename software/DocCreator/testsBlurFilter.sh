#!/bin/sh

heure=$(date +%Hh%M)
jour=$(date +%Y-%m-%d)
mkdir "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour"

writeDiv(){

    echo " <div>\n <p>Méthode $1, avec un filtre de taille ($2,$3)</p>\n <ul>\n
<li><p>Image source</p>\n <img id=\"src$6\" src=\"$4\" style=\"width:250px; height:400px\"></li>
 <li><p>Image résultat</p>\n <img id=\"res$6\" src=\"$5\" style=\"width:250px; height:400px\"></li>\n </div>" >> ${HOME}/testDegradations/Flou/$heure\_$jour.html

}

#Ecriture de l'entete
echo "<!doctype html>  \n\n <head>\n <meta charset=\"utf-8\">
 <title> Test du $jour à $heure</title>\n <script type=\"text/javascript\" src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.4.2/jquery.min.js\"></script> \n <script type=\"text/javascript\" src=\"zoomer.js\"></script> \n <script type=\"text/javascript\">
jQuery(document).ready(function($){
 \$('#src1').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#res1').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#src2').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#res2').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#src3').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#res3').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#src4').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
 \$('#res4').addpowerzoom({
		defaultpower: 2,
		powerrange: [2,10],
		largeimage: null,
		magnifiersize: [100,100]
	});
})
</script>\n  </head>\n\n <body>\n
 <h1>Test du $jour à $heure</h1>" >> ${HOME}/testDegradations/Flou/$heure\_$jour.html

#Premier Test
${HOME}/doccreator/build/software/AncientDocumentEditor/mySoft ${HOME}/testDegradations/Flou/imageTest/BMP72_0004.jpg 0 9 3 "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/1.jpg"

writeDiv "Gaussian" 5 7 "${HOME}/testDegradations/Flou/imageTest/BMP72_0004.jpg" "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/1.jpg" 1

#Deuxieme Test
${HOME}/doccreator/build/software/AncientDocumentEditor/mySoft ${HOME}/testDegradations/Flou/imageTest/BMP72_0007.jpg 1 9 9 "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/2.jpg"

writeDiv "Median" 4 4 "${HOME}/testDegradations/Flou/imageTest/BMP72_0007.jpg" "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/2.jpg" 2

#Troisième Test
${HOME}/doccreator/build/software/AncientDocumentEditor/mySoft ${HOME}/testDegradations/Flou/imageTest/CESR_287_0006.jpg 2 9 1 "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/3.jpg"

writeDiv "Normal" 3 3 "${HOME}/testDegradations/Flou/imageTest/CESR_287_0006.jpg" "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/3.jpg" 3

#Quatrième Test
${HOME}/doccreator/build/software/AncientDocumentEditor/mySoft ${HOME}/testDegradations/Flou/imageTest/CESR_287_0010.jpg 2 9 9 "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/4.jpg"

writeDiv "Normal" 3 7 "${HOME}/testDegradations/Flou/imageTest/CESR_287_0010.jpg" "${HOME}/testDegradations/Flou/imageRes/$heure\_$jour/4.jpg" 4

#Ecriture fin de fichier html
echo "\n </body>\n\n</html>" >> ${HOME}/testDegradations/Flou/$heure\_$jour.html
