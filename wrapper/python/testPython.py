import DocCreator
import cv2

img = cv2.imread("/home/mansenca/SAI/DOCUMENT/DocCreator_git/tests/Degradations/testBleedThroughData/in2Color.png")
img2 = cv2.imread("/home/mansenca/SAI/DOCUMENT/DocCreator_git/tests/Degradations/testBleedThroughData/in2Color.png")

PHANTOM_PATH="/home/mansenca/SAI/DOCUMENT/DocCreator_git/data/Image/phantomPatterns"
STAINS_PATH="/home/mansenca/SAI/DOCUMENT/DocCreator_git/data/Image/stainImages/images"
HOLE_PATTERN="/home/mansenca/SAI/DOCUMENT/DocCreator_git/data/Image/holePatterns/borderHoles/pattern8.png"

#TODO
#- read images in other formats: CV_8UC1 ? CV_8UC4 ?
#     cv2.IMREAD_COLOR (default), cv2.IMREAD_GRAYSCALE, cv2.IMREAD_UNCHANGED


img3 = DocCreator.grayscaleCharsDegradation(img, level=5, I=33.0, O=33.0, D=34.0)
img3 = DocCreator.bleedThrough(img3, img2, nbIter=30, x=20, y=20, nbThreads=-1)

img3 = DocCreator.phantomCharacter(img3, frequency=2, phantomPatternsPath=PHANTOM_PATH)

img3 = DocCreator.gradientDomainDegradation(img3, stainImagePath=STAINS_PATH, numStainsToInsert=8, insertType=2, doRotations=True)


holeImg = cv2.imread(HOLE_PATTERN, cv2.IMREAD_GRAYSCALE)
#holeImg = cv2.imread(HOLE_PATTERN) #TODO: be able to use a CV_8UC3 holeImg ???
img3 = DocCreator.holeDegradation(img3, holeImg,
                                  xOrigin=img.shape[0]/2, yOrigin=img.shape[1]/2,
                                  size=1, holeType=1, side=1, color=[220, 60, 60])#, shadowBorderWidth=0, shadowBorderIntensity=10.0)

img3 = DocCreator.blur(img3, method=0, intensity=5)

img3 = DocCreator.shadowBinding(img3, border=3, distance=img.shape[0]/3, intensity=0.9, angle=80)


output="degraded.png"
cv2.imwrite(output, img3)

