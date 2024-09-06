import math
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

def rotate_point(p0, angleInRads, p ):
    s = math.sin(angleInRads)
    c = math.cos(angleInRads)

  #translate point back to origin:
    p.x -= p0.x
    p.y -= p0.y

  # rotate point
    xnew = p.x * c - p.y * s
    ynew = p.x * s + p.y * c

  # translate point back:
    p.x = xnew + p0.x
    p.y = ynew + p0.y

    return p


parts = 3

startPos = Point(0, 0)

point = Point(23, 40)
rotation = [60, 115]
print("Point", point.x - startPos.x, point.y - startPos.y, "Rotation from", rotation[0], "to", rotation[1])
degStep = (rotation[1]-rotation[0])/(parts + 1)
print(degStep)
for i in range(parts + 1):
    rotated = rotate_point(startPos, math.radians(degStep), point)
    print(rotated.x, rotated.y)