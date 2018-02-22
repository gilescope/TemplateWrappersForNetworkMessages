#![feature(alloc,global_allocator, allocator_api, heap_api)]
//#![no_std]
#![feature(test)]
#[macro_use] extern crate serde_derive;
extern crate serde;
extern crate alloc;
extern crate rmp_serde as rmps;
extern crate rmpv;
extern crate jemallocator;
extern crate test;

use serde::{Deserialize, Serialize};
use rmps::{Deserializer, Serializer};

use jemallocator::Jemalloc;

//Just for Capnproto:
extern crate capnp;
extern crate failure;
pub mod message_capnp {
    include!("../schema/message_capnp.rs");
}
mod test_capnproto;
//end just for capnproto

#[global_allocator]
static A: Jemalloc = Jemalloc;

#[derive(Serialize, Deserialize, Debug)]
struct Header {
    source: [char; 4],
    type_id: usize,
}

#[derive(Serialize, Deserialize, Debug)]
struct Message4 {
    header: Header,
    message4_text: String,
    an_int: i32,
}

pub fn main()
{
    let a = Message4{
        header: Header{ source: ['F','r','e','d'], type_id: 4 },
        message4_text: String::from("This is a variable length string"),
        an_int: 126
    };

    ben(&a);

    println!("Hello");
    std::process::exit(0);
}

fn ben(a: &Message4)
{
    for _i in 0..10_000 {
        // 'Send' this message:
        let mut data = Vec::new();
        {
            let mut serialiser = Serializer::new(&mut data);
            a.serialize(&mut serialiser).unwrap();
        }
        let _size: usize = data.len();

        //      'Receive' the message:
        let mut de = Deserializer::new(&data[..]);
        let mut received_msg: Message4 = Deserialize::deserialize(&mut de).unwrap();

        // Translate the received message to the correct type:
        if received_msg.header.type_id == 4 {
            received_msg.header.type_id = 16;
            received_msg.header.source = ['J', 'a', 'n', 'e'];
            // Use the content of the received msg:
            received_msg.an_int = 128;
        }
        //data.clear();
    }
}


#[cfg(test)]
mod tests {
    use test::{Bencher};

    #[bench]
    fn bench_pow(b: &mut Bencher) {
        let a = super::Message4{
            header: super::Header{ source: ['F','r','e','d'], type_id: 4 },
            message4_text: String::from("This is a variable length string"),
            an_int: 126
        };

        b.iter(|| {
            // Inner closure, the actual test
            super::ben(&a);
        });
    }
}