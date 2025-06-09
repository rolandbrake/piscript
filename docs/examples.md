# PiScript Examples

This section provides practical examples demonstrating how to use PiScript features and built-in functions. These examples cover different categories, including basic syntax, arrays, functions, and matrix operations.

---

## Hello World

```piscript
println("Hello, PiScript!")


```

---

## Fibonacci series

```piscript

fun fib(n = 0) {
    if(n <= 1)
        return n;
    return fib(n-1) + fib(n-2);
}

println(fib(10));

fun fib(n = 0) {    
  
    if(n <= 1)
      return n;      
  
    let _fib = 1;
    let prevFib = 1;
    let temp;
  
    for(i in 2..n) {
      temp = _fib;
      _fib += prevFib;
      prevFib = temp;
    }
    return _fib;
  }
  
  for(i in 0..40)
    println(fib(i) + " ");




```

---

## Factorial

```piscript
fun factorial(n = 0) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

for (i in 1..20) 
    println("Factorial of " + i + " is " + factorial(i));


```

---

## Binary Search

```piscript
fun binary_search(l = [], value =  0) {
    let low = 0;
    let high = #l - 1;
    while(low <= high) {
        let mid = floor((low+high) / 2);
        if(l[mid] > value) high = mid - 1;
        elif(l[mid] < value) low = mid + 1;
        else return mid;
    }
    return -1;
}

for(i in 1..6)
    println(binary_search([1,2,3,4,5], i));



```

---

## Bubble Sort

```piscript
fun bubbleSort(list =  []) {
    let n = #list;    
    let swapped = false;
    for (i in 0..n) {             
        for (j in 0..n - i) {                            
            if (list[j] > list[j + 1]) {
                let temp = list[j];                                                       
                list[j] = list[j + 1];                                     
                list[j + 1] = temp;   
                swapped = true;                          
            }
        }
        if (!swapped)
            break;
    }
    return list;
  }
  
  println(bubbleSort([5,4,3,2,1]));
```


---


## Selection Sort

```piscript

// Sort an array (or list) of elements using the Selection sort algorithm.
fun selectionSort(nums) {
  let len = #nums;    
  for(i in 0..len) {      
    let minAt = i;
    for(j in i+1..len) {
      if(nums[j] < nums[minAt])
        minAt = j;
    }

    if(minAt != i) {
      let temp = nums[i];
      nums[i] = nums[minAt];
      nums[minAt] = temp;        
    }    
  }    
  return nums;
}

list = [94, 7, 127, 85, 186, 55, 138,200, 168, 15, 39, 120, 26, 176, 161, 62, 102, 50, 95, 121, 153, 198, 19, 76, 87, 44, 81, 36, 92, 70, 141, 66, 122, 78, 42, 148, 28, 114, 65, 10, 193, 89, 4, 183, 169, 47, 186, 118, 30, 101, 54, 174, 33, 187, 57, 143, 16, 72, 195];

println(selectionSort(list[0:1000]));

```

---

## Random Pixels Generation

```piscript
while (true) {
  let i = 0
  while (i < 100000) {
    pixel(rand() * 128, rand() 
    * 128, rand() * 16 + 1)
    i++
  }
  draw()
}


```

---

## FizzBuzz

```piscript
for(i in 1..101)
    if(i % 15 == 0)
        println("FizzBuzz");
    elif(i % 3 == 0)
        println("Fizz");
    elif(i % 5 == 0)
        println("Buzz");
    else
        println(i);
```

---


## Sum of List

```piscript
fun sum(list = []) {
    let res = 0;
    for(l in list)
        res += l;
    return res;
}


println(sum([5,4,3,2,1]));
```

---


## Leonardo numbers


```piscript
fun Leonardo(l_zero, l_one, add, amount){
    let terms = [l_zero,l_one], new    
    while #terms < amount {
        new = terms[-1] + terms[-2]        
        new += add        
        terms = terms + new        
    }
    return terms
}

out = ""
println("First 25 Leonardo numbers:")
for term in Leonardo(1,1,1,25)
    out += term + " "

println(out)

out = ""
println("Leonardo numbers with fibonacci parameters:")
for term in Leonardo(0,1,0,25)
    out += term + " "

println(out)
```

---


## Linear Regression

```piscript
// Sample data: x and y values
x = [3, 11, 18, 25, 31, 39, 46, 52, 58, 63, 68, 73, 78, 83, 89, 95, 102, 111, 119, 126]
y = [5, 17, 24, 29, 37, 41, 49, 54, 60, 66, 71, 76, 81, 86, 92, 97, 104, 112, 121, 127]



// Compute means
n = #x
sum_x = 0
sum_y = 0
for i in 0..n {
    sum_x += x[i]
    sum_y += y[i]
}
mean_x = sum_x / n
mean_y = sum_y / n

// Compute slope (m) and intercept (b) for y = mx + b
num = 0
den = 0
for i in 0..n {
    dx = x[i] - mean_x
    dy = y[i] - mean_y
    num += dx * dy
    den += dx * dx
}
m = num / den
b = mean_y - m * mean_x

// Function to predict y from x using the regression line
predict = (x) -> m * x + b

// Clear screen (assuming pixel function and clear function available)
clear()


// Plot regression line in color 8 (light blue)
for i in 0..128 {
    yi = predict(i)
    if yi >= 0 && yi < 128 {
        pixel(i, 128 - yi, 8)
    }
}

// Plot data points as white (6)
for i in 0..n {    
    px = x[i]
    py = 128 - y[i]  // Flip y for screen coordinates    
    pixel(px, py, 6)
}

// Optionally print slope and intercept
println("m =", m)
println("b =", b)

draw()

```

---


## Check if Prime

```piscript

fun isPrime(n){       
    for(i in 2..floor(n**0.5) + 1)
        if(n % i == 0)
            return false;        
    return true;
}

for(i in 2..200){
    if(isPrime(i))
        println(i);
}




fun isPrime(num = 0) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (i in 5..num:2)
        if (num % i == 0) return false;
    return true;
}

let sum = 0;

for(i in 1..100000){
    if(isPrime(i))
        sum += i;
}


println("Sum of primes up to 10000: " + sum);




```

---


## Clousres

```piscirpt


fun make_counter() {
    let i = 0;
    fun count() {
      i = i + 1;
      println(i);
    }
  
    return count;
  }
  
  let counter = make_counter();
  counter(); // "1".
  counter(); // "2".


```


---

## Weiferich Number


```piscript
fun isPrime(n) {   
    for i in 2..floor(n**0.5) + 1
        if(n % i == 0)
            return false        
    return true
}

fun isWeiferich(p) {
    if !isPrime(p)
        return false    
    let q = 1
    let p2 = p ** 2    
    while p > 1 {
        q = (2 * q) % p2
        p -= 1
    }
    if q == 1
        return true
    else
        return false
}

println("Wieferich primes less than 5000: ")
i = 2
while i <= 5000 {
    if isWeiferich(i)
        println(i)
    i++
}

```


## ROT13

```piscirpt
let LETTERS = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';


fun rot13(message = '') {
    let res = "";
    for(ch in message) {
        let i = 0;
        for(l in LETTERS) {                                             
            if(l == ch) {         
                if(i < 26)   
                    res += LETTERS[(i + 13) % 26];   
                else
                    res += LETTERS[((i + 13) % 26) + 26];             
                break;
            }
            i++;            
        }
    }
    return res;
}

println(rot13("YsREgKcTNODgGwvChyXqDgFJwCVQGmJpAAZUAAHLpMjmtdPVScwoKUctXbYeCHFFJwECJuLODFdssPQhdxxOyMXBDAYUDGtjnr"));

```

---



These examples illustrate the expressive and concise syntax of PiScript. More advanced examples will be added soon.
