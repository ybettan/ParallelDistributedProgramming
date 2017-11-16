package ex1;

/*
 * Class Neighbors is used to help a thread (field) to communicate with threads around.
 * Specificly to send copys of Cells after they do an update.
 */
public class Neighbors {
    Field up, upRight, right, downRight, down, downLeft, left, upLeft;

    public Neighbors(Field up, Field upRight, Field right, Field downRight, Field down,
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

    public Field getUp() {
        return up;
    }

    public Field getUpRight() {
        return upRight;
    }

    public Field getRight() {
        return right;
    }

    public Field getDownRight() {
        return downRight;
    }

    public Field getDown() {
        return down;
    }

    public Field getDownLeft() {
        return downLeft;
    }

    public Field getLeft() {
        return left;
    }

    public Field getUpLeft() {
        return upLeft;
    }


}
