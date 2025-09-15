// 1. Node Definition
class BidirectionalNode<E> {
    public E item;
    public BidirectionalNode<E> next;
    public BidirectionalNode<E> prev;

    public BidirectionalNode(E newItem) {
        this.item = newItem;
        this.next = this.prev = null;
    }
}

// 2. Main Test Class
public class ExceptionTest<E extends Comparable<E>> {
    private BidirectionalNode<E> head;

    public ExceptionTest() {
        // The dummy head for the circular list
        head = new BidirectionalNode<>(null);
        head.next = head;
        head.prev = head;
    }

    // 3. The original buggy add() method
    public void add(E x) {
        BidirectionalNode<E> curr = head.next;

        // *** An error will occur here (no null check) ***
        while (curr != head && curr.item.compareTo(x) < 0) {
            curr = curr.next;
        }

        BidirectionalNode<E> prevNode = curr.prev;
        BidirectionalNode<E> newNode = new BidirectionalNode<>(x);
        prevNode.next = newNode;
        newNode.prev = prevNode;
        newNode.next = curr;
        curr.prev = newNode;
    }

    public void display() {
        System.out.print("List: ");
        BidirectionalNode<E> curr = head.next;
        while (curr != head) {
            System.out.print(curr.item + " -> ");
            curr = curr.next;
        }
        System.out.println("END");
    }

    // 4. Test execution code
    public static void main(String[] args) {
        ExceptionTest<Integer> list = new ExceptionTest<>();
        list.add(10);
        list.add(30);

        // Manually insert a node with a null item to trigger the exception
        BidirectionalNode<Integer> firstNode = list.head.next;
        BidirectionalNode<Integer> thirdNode = firstNode.next;
        BidirectionalNode<Integer> nullItemNode = new BidirectionalNode<>(null);
        firstNode.next = nullItemNode;
        nullItemNode.prev = firstNode;
        nullItemNode.next = thirdNode;
        thirdNode.prev = nullItemNode;
        
        System.out.println("--- List state before the error ---");
        list.display();
        System.out.println("\nAn error will occur when calling add(20)...");

        // This call will cause a NullPointerException
        list.add(20);
    }
}