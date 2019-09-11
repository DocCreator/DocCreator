import Degradations

def bleedThrough(imgRecto, imgVerso, nbIter, x=0, y=0, nbThreads=-1):
    imgOutLength = imgRecto.shape[0] * imgRecto.shape[1] * imgRecto.shape[2]
    res = Degradations.bleedThrough(imgOutLength, imgRecto, imgVerso, nbIter, x, y, nbThreads)
    return res.reshape(imgRecto.shape)

def blur(img, method, intensity):
    imgOutLength = img.shape[0] * img.shape[1] * img.shape[2]
    res = Degradations.blur(imgOutLength, img,
                            method, intensity)
    return res.reshape(img.shape)


def blurArea(img, method, intensity, function, area=0, coeff=1.0, vertical=0, horizontal=0, radius=0):
    imgOutLength = img.shape[0] * img.shape[1] * img.shape[2]
    res = Degradations.blurArea(imgOutLength, img,
                                method, intensity,
                                function, area, coeff, vertical, horizontal, radius)
    return res.reshape(img.shape)


def gradientDomainDegradation(img, stainImagePath, numStainsToInsert, insertType=2, doRotations=True):
    imgOutLength = img.shape[0] * img.shape[1] * img.shape[2]
    res = Degradations.gradientDomainDegradation(imgOutLength, img,
                                                 stainImagePath, numStainsToInsert, insertType, doRotations)
    return res.reshape(img.shape)


def grayscaleCharsDegradation(img, level, I=33.0, O=33.0, D=34.0):
    imgOutLength = img.shape[0] * img.shape[1] * img.shape[2]
    res = Degradations.grayscaleCharsDegradation(imgOutLength, img, level, I, O, D)
    return res.reshape(img.shape)


def holeDegradation(img, patternImg, xOrigin, yOrigin, size, holeType, side, color, belowImg=None, shadowBorderWidth=0, shadowBorderIntensity=1000):
    imgOutLength = img.shape[0] * img.shape[1] * img.shape[2]
    if (belowImg is None):
        res = Degradations.holeDegradation(imgOutLength, img,
                                           patternImg, xOrigin, yOrigin, size, holeType, side, color)
        #TODO: shadowBorderWidth, shadowBorderIntensity: are currently not used when belowImg is not used
        # If we pass them, there is a SWIG ambiguity...
    else:
        res = Degradations.holeDegradation(imgOutLength, img,
                                           patternImg, xOrigin, yOrigin, size, holeType, side, color,
                                           belowImg, shadowBorderWidth, shadowBorderIntensity)
    return res.reshape(img.shape)


def phantomCharacter(img, frequency, phantomPatternsPath):
    imgOutLength = img.shape[0] * img.shape[1] * img.shape[2]
    res = Degradations.phantomCharacter(imgOutLength, img,
                                        frequency, phantomPatternsPath)
    return res.reshape(img.shape)


def shadowBinding(img, border, distance, intensity, angle):
    imgOutLength = img.shape[0] * img.shape[1] * img.shape[2]
    res = Degradations.shadowBinding(imgOutLength, img,
                                     border, distance, intensity, angle)
    return res.reshape(img.shape)
