use std::net::{TcpStream, TcpListener};
use std::io::{Read, Write};

fn main() {
    // Connecting to a server
    let mut stream = TcpStream::connect("127.0.0.1:8100").expect("Failed to connect");
    
    // Sending data
    stream.write(b"Hello, TCP!").expect("Failed to write to stream");
    
    // Receiving data
    let mut buffer = [0; 512];
    let bytes_read = stream.read(&mut buffer).expect("Failed to read from stream");
    println!("Received: {}", String::from_utf8_lossy(&buffer[..bytes_read]));

    // Closing the connection
    drop(stream);  // or stream.shutdown()
}
