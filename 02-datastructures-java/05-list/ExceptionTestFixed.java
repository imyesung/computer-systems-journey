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

// 2. The Fixed Test Class
public class ExceptionTestFixed<E extends Comparable<E>> {
    private BidirectionalNode<E> head;

    // Constructor name is also changed
    public ExceptionTestFixed() {
        head = new BidirectionalNode<>(null); // Dummy head
        head.next = head;
        head.prev = head;
    }

    // 3. The fixed add() method
    public void add(E x) {
        BidirectionalNode<E> curr = head.next;

        // *** This is the fixed line with the null check ***
        while (curr != head && curr.item != null && curr.item.compareTo(x) < 0) {
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

    // 4. Test execution code for the fixed version
    public static void main(String[] args) {
        // Create an instance of the fixed class
        ExceptionTestFixed<Integer> list = new ExceptionTestFixed<>();
        list.add(10);
        list.add(30);

        // Manually insert a node with a null item
        BidirectionalNode<Integer> firstNode = list.head.next;
        BidirectionalNode<Integer> thirdNode = firstNode.next;
        BidirectionalNode<Integer> nullItemNode = new BidirectionalNode<>(null);
        firstNode.next = nullItemNode;
        nullItemNode.prev = firstNode;
        nullItemNode.next = thirdNode;
        thirdNode.prev = nullItemNode;
        
        System.out.println("--- List state before adding 20 ---");
        list.display();

        System.out.println("\nCalling add(20)... This will now succeed.");
        // This call will now work without an exception
        list.add(20);
        
        System.out.println("\n--- Final list state ---");
        list.display();
    }
}