<h1>CUT (CPU Usage Tracker)</h1>

<p>CUT is a multi-threaded C++ application designed to track and analyze CPU usage on a system. It reads data from the "/proc/stat" file, processes the information, and displays the CPU usage for each CPU core.</p>

<h2>Prerequisites</h2>

<p>Before running the application, ensure that you have the following dependencies installed:</p>

<ul>
  <li>C++ compiler supporting C++20</li>
  <li>CMake version 3.25 or higher</li>
  <li>ncurses library</li>
</ul>

<h2>Usage</h2>

<p>To use it, follow these steps:</p>

<ol>
  <li>Create a build directory:</li>
  <h3></h3>
  <pre><code>mkdir build
cd build</code></pre>
  <h3></h3>
  <li>Generate the build files using CMake:</li>
  <h3></h3>
  <pre><code>cmake ..</code></pre>
  <h3></h3>
  <li>Build the application:</li>
  <h3></h3>
  <pre><code>make</code></pre>
  <h3></h3>
  <li>Run the compiled executable:</li>
  <h3></h3>
  <pre><code>./CUT</code></pre>
  <h3></h3>
</ol>

<h2>Code Overview</h2>

<p>The application consists of several classes:</p>

<ul>
  <li>Reader: Reads data from the /proc/stat file and pushes it into a buffer for analysis.</li>
  <li>Logger: Receives messages from all threads and writes them into a log file.</li>
  <li>Printer: Prints the CPU usage information to the console.</li>
  <li>Analyzer: Analyzes CPU data received from the reader and calculates the CPU usage.</li>
</ul>

<p>The <code>main()</code> function creates instances of these classes, starts the corresponding threads, and manages the termination process. It also registers a signal handler to handle the SIGINT signal (generated when the user interrupts the program).</p>

<p>Each class has its own implementation file (Reader.cpp, Logger.cpp, Printer.cpp, and Analyzer.cpp) that defines the member functions and implements the necessary logic.</p>

<h2>Thread Communication</h2>

<p>The communication between threads is achieved using lock-free SPSC (Single Producer Single Consumer) queues. These queues act as buffers for passing data between the reader, logger, analyzer, and printer threads.</p>

<ul>
  <li>The Reader thread reads data from the /proc/stat file and pushes it into the logger and analyzer buffers.</li>
  <li>The Logger thread receives messages from all threads and writes them into a log file.</li>
  <li>The Analyzer thread analyzes the CPU data received from the reader and calculates the CPU usage, pushing it into the printer buffer.</li>
  <li>The Printer thread prints the CPU usage information to the console.</li>
</ul>

<h2>Termination</h2>

<p>The termination of the application is controlled by a termination flag (<code>terminate</code>). When the SIGINT signal is received (e.g., by pressing CTRL+C), the signal handler sets the termination flag to true, and the main thread stops all the worker threads. This ensures a graceful shutdown of the application.</p>

<h2>Contributing</h2>

<p>Contributions to the project are welcome. If you find any issues or have suggestions for improvements, please create a GitHub issue or submit a pull request.</p>

<h2>License</h2>

<p>The code is open source and available under the MIT License. Feel free to modify and distribute it as needed.</p>
