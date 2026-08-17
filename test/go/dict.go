package main

import (
	"os"
	"fmt"
	"time"
	"strings"
	"strconv"
	"bufio"
)

func main() {
	var N, _ = strconv.Atoi(os.Args[1])
	t0 := time.Now()
	fmt.Println("N=", N)
	var m = make(map[string]int)

	for i := 0; i < N; i++ {
		m[strconv.Itoa(i)] = 0
	}
	t1 := time.Since(t0)
	t2 := time.Now()

	var ok bool
	for i := 0; i < N; i++ {
		_, ok = m[strconv.Itoa(i)]
		if ok == false { panic("something is broken"); }
	}

	t3 := time.Since(t2)

	fmt.Println("Insert:", t1.Milliseconds(), "ms")
	fmt.Println("Scan:", t3.Milliseconds(), "ms")
	fmt.Println("Total:", (t1 + t3).Milliseconds(), "ms")

	f3, _ := os.Open("/proc/self/status")
	defer f3.Close()
	scanner3 := bufio.NewScanner(f3)
	for scanner3.Scan() {
		var line = scanner3.Text()
		if strings.Contains(line, "VmHWM") {
			fmt.Println(line)
		}
	}
}
