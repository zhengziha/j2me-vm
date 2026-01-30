public class PolymorphismTest {
    public static void main(String[] args) {
        System.out.println("=== Polymorphism Test ===");
        
        // 创建父类引用指向子类实例
        Animal animal = new Dog();
        System.out.println("Animal reference to Dog:");
        animal.makeSound(); // 应该调用Dog的实现
        
        // 创建另一个子类实例
        animal = new Cat();
        System.out.println("Animal reference to Cat:");
        animal.makeSound(); // 应该调用Cat的实现
        
        // 直接使用子类引用
        Dog dog = new Dog();
        System.out.println("Dog reference to Dog:");
        dog.makeSound(); // 应该调用Dog的实现
        
        Cat cat = new Cat();
        System.out.println("Cat reference to Cat:");
        cat.makeSound(); // 应该调用Cat的实现
        
        System.out.println("=== Polymorphism Test Completed ===");
    }
}

class Animal {
    public void makeSound() {
        System.out.println("Animal makes a sound");
    }
}

class Dog extends Animal {
    @Override
    public void makeSound() {
        System.out.println("Dog barks: Woof! Woof!");
    }
}

class Cat extends Animal {
    @Override
    public void makeSound() {
        System.out.println("Cat meows: Meow! Meow!");
    }
}
