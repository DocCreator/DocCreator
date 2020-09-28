if __package__ or "." in __name__:
    from . import Degradations
else:
    import Degradations
#import Degradations

def bleedThrough(imgRecto, imgVerso, nbIter, x=0, y=0, nbThreads=-1):
    imgOutLength = imgRecto.size
    res = Degradations.bleedThrough(imgOutLength, imgRecto, imgVerso, nbIter, x, y, nbThreads)
    return res.reshape(imgRecto.shape)

def blur(img, method, intensity):
    imgOutLength = img.size
    res = Degradations.blur(imgOutLength, img,
                            method, intensity)
    return res.reshape(img.shape)


def blurArea(img, method, intensity, function, area=0, coeff=1.0, vertical=0, horizontal=0, radius=0):
    imgOutLength = img.size
    res = Degradations.blurArea(imgOutLength, img,
                                method, intensity,
                                function, area, coeff, vertical, horizontal, radius)
    return res.reshape(img.shape)


def gradientDomainDegradation(img, stainImagePath, numStainsToInsert, insertType=2, doRotations=True):
    imgOutLength = img.size
    res = Degradations.gradientDomainDegradation(imgOutLength, img,
                                                 stainImagePath, numStainsToInsert, insertType, doRotations)
    return res.reshape(img.shape)


def grayscaleCharsDegradation(img, level, I=33.0, O=33.0, D=34.0):
    imgOutLength = img.size
    res = Degradations.grayscaleCharsDegradation(imgOutLength, img, level, I, O, D)
    return res.reshape(img.shape)


# def holeDegradation(img, patternImg, xOrigin, yOrigin, size, holeType, side, color, belowImg=None, shadowBorderWidth=0, shadowBorderIntensity=1000):
#     imgOutLength = img.size
#     if (belowImg is None):
#         res = Degradations.holeDegradation(imgOutLength, img,
#                                            patternImg, xOrigin, yOrigin, size, holeType, side, color)
#         #TODO: shadowBorderWidth, shadowBorderIntensity: are currently not used when belowImg is not used
#         # If we pass them, there is a SWIG ambiguity...
#     else:
#         res = Degradations.holeDegradation(imgOutLength, img,
#                                            patternImg, xOrigin, yOrigin, size, holeType, side, color,
#                                            belowImg, shadowBorderWidth, shadowBorderIntensity)
#     return res.reshape(img.shape)

def holeDegradation(img, patternImg, xOrigin, yOrigin, size, holeType, side, color):
    imgOutLength = img.size
    res = Degradations.holeDegradation(imgOutLength, img,
                                       patternImg, xOrigin, yOrigin, size, holeType, side, color)
    return res.reshape(img.shape)

def holeDegradation2(img, patternImg, xOrigin, yOrigin, size, holeType, side, color, belowImg, shadowBorderWidth=0, shadowBorderIntensity=1000):
    imgOutLength = img.size
    res = Degradations.holeDegradation(imgOutLength, img,
                                       patternImg, xOrigin, yOrigin, size, holeType, side, color,
                                       belowImg, shadowBorderWidth, shadowBorderIntensity)
    return res.reshape(img.shape)





def phantomCharacter(img, frequency, phantomPatternsPath):
    imgOutLength = img.size
    res = Degradations.phantomCharacter(imgOutLength, img,
                                        frequency, phantomPatternsPath)
    return res.reshape(img.shape)


def shadowBinding(img, border, distance, intensity, angle):
    imgOutLength = img.size
    res = Degradations.shadowBinding(imgOutLength, img,
                                     border, distance, intensity, angle)
    return res.reshape(img.shape)


def degrade3D(img, meshFilename, random=True):
    imgOutLength = img.size
    res = Degradations.degrade3D(imgOutLength, img, meshFilename, random)
    return res.reshape(img.shape)
    	       
def degrade3DWithBackground(img, meshFilename, imgBackground, random=True):
    imgOutLength = img.size
    res = Degradations.degrade3DWithBackground(imgOutLength, img, meshFilename, random)
    return res.reshape(img.shape)


def addGaussianNoise(img, average=0.0, standard_deviation=10.0, addType=0):
    imgOutLength = img.size
    res = Degradations.addGaussianNoise(imgOutLength, img, average, standard_deviation, addType)
    return res.reshape(img.shape)

def addSpeckleNoise(img, average=0.0, standard_deviation=10.0, addType=0):
    imgOutLength = img.size
    res = Degradations.addSpeckleNoise(imgOutLength, img, average, standard_deviation, addType)
    return res.reshape(img.shape)

def addSaltAndPepperNoise(img, amount=0.15, ratio=0.5):
    imgOutLength = img.size
    res = Degradations.addSaltAndPepperNoise(imgOutLength, img, amount, ratio)
    return res.reshape(img.shape)
   
def rotateFillColor(img, angle, color):
    imgOutLength = img.size
    res = Degradations.rotateFillColor(imgOutLength, img, angle, color)
    return res.reshape(img.shape)

def rotateFillImage(img, angle, imgBackground):
    imgOutLength = img.size
    res = Degradations.rotateFillImage(imgOutLength, img, angle, imgBackground)
    return res.reshape(img.shape)

def rotateFillImageRepeats(img, angle, imgBackground, repeats):
    imgOutLength = img.size
    res = Degradations.rotateFillImageRepeats(imgOutLength, img, angle, imgBackground, repeats)
    return res.reshape(img.shape)

def rotateFillBorder(img, angle, border):
    imgOutLength = img.size
    res = Degradations.rotateFillBorder(imgOutLength, img, angle, border)
    return res.reshape(img.shape)

def rotateFillInpaint1(img, angle, inpaintingRatio = 0.05):
    imgOutLength = img.size
    res = Degradations.rotateFillInpaint1(imgOutLength, img, angle, inpaintingRatio)
    return res.reshape(img.shape)

def rotateFillInpaint2(img, angle, imgBackground):
    imgOutLength = img.size
    res = Degradations.rotateFillInpaint2(imgOutLength, img, angle, imgBackground)
    return res.reshape(img.shape)

def rotateFillInpaint3(img, angle, imgBackground):
    imgOutLength = img.size
    res = Degradations.rotateFillInpaint3(imgOutLength, img, angle, imgBackground)
    return res.reshape(img.shape)

def elasticDeformation(img, alpha=2.0, sigma=0.08, border=4, interpolation=1):
    imgOutLength = img.size
    res = Degradations.elasticDeformation(imgOutLength, img, alpha, sigma, border, interpolation)
    return res.reshape(img.shape)

def elasticDeformation2(img, alpha=2.0, sigma=0.08, alpha_affine=9.0, border=4, interpolation=1):
    imgOutLength = img.size
    res = Degradations.elasticDeformation2(imgOutLength, img, alpha, sigma, alpha_affine, border, interpolation)
    return res.reshape(img.shape)

