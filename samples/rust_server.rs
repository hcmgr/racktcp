use std::net::{TcpListener, TcpStream};
use std::io::{Read, Write};

fn main() {
    let listener = TcpListener::bind("127.0.0.1:8100").expect("Failed to bind to address");
    
    for stream in listener.incoming() {
        let mut stream = stream.expect("Failed to accept connection");
        
        // Receiving data
        let mut buffer = [0; 512];
        let bytes_read = stream.read(&mut buffer).expect("Failed to read from stream");
        println!("Received: {}", String::from_utf8_lossy(&buffer[..bytes_read]));

        // Sending data
        stream.write(b"Hello from server!").expect("Failed to write to stream");
    }
}
