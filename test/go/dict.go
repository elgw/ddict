package main

// go run .
// compile with
// $ go build

import (
	"os"
	"log"
	"bufio"
	"fmt"
	"time"
	//"regexp"
	"strings"
)

func main() {
	f, err := os.Open("../../dictwords.txt")

	if err != nil {
		log.Fatal(err)
	}

	defer f.Close()

	scanner := bufio.NewScanner(f)

	t0 := time.Now()

	// Create a dictionary
	var m = make(map[string]int)

	// Add all words
	for scanner.Scan() {
		var line = scanner.Text()
		m[line] = 1
	}
	t1 := time.Since(t0)
	fmt.Println("Construction took", t1)

	fmt.Println(len(m), " words added to the dictionary")

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

	f2, err := os.Open("../../text.txt")

	if err != nil {
		log.Fatal(err)
	}

	defer f2.Close()

	scanner2 := bufio.NewScanner(f2)

	var n_known = 0
	var n_unknown = 0
	var n_words = 0
	t2 := time.Now()

	for scanner2.Scan() {
		var line = scanner2.Text()
		words := strings.Fields(line)
		for _, word := range words{
			n_words = n_words + 1
			if m[word] == 1 {
				n_known = n_known + 1
			} else {
				n_unknown = n_unknown + 1
			}
		}
	}
	t3 := time.Since(t2)
	fmt.Println("Scanning took", t3)
	fmt.Println("n_words ", n_words)
	fmt.Println("Known: ", n_known, " unknown: ", n_unknown)
	fmt.Println("Total time:", t1 + t3)
}
