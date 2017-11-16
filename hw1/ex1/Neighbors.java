package ex1;

/*
 * Class Neighbors is used to help a thread (field) to communicate with threads around.
 * Specificly to send copys of Cells after they do an update.
 */
public class Neighbors {
    Field up, upRight, right, downRight, down, downLeft, left, upLeft;

    Neighbors(Field up, Field upRight, Field right, Field downRight, Field down,
              Field downLeft, Field left, Field upLeft) {
        this.up = up;
        this.upRight = upRight;
        this.right = right;
        this.downRight = downRight;
        this.down = down;
        this.downLeft = downLeft;
        this.left = left;
        this.upLeft = upLeft;
    }

    Field getUp() {
        return up;
    }

    Field getUpRight() {
        return upRight;
    }

    Field getRight() {
        return right;
    }

    Field getDownRight() {
        return downRight;
    }

    Field getDown() {
        return down;
    }

    Field getDownLeft() {
        return downLeft;
    }

    Field getLeft() {
        return left;
    }

    Field getUpLeft() {
        return upLeft;
    }


}
