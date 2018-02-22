use std;

use serde::{Deserialize, Serialize};
use rmps::{Deserializer, Serializer};

use message_capnp::*;
use capnp;
use capnp::serialize;
use failure::Error;
use std::io::Cursor;

pub fn main()
{
    let mut message = ::capnp::message::Builder::new_default();
    {
        let mut a = message.init_root::<message4::Builder>();
        a.set_an_int(126);
        a.set_message("This is a variable length string");
        let mut header_builder = a.init_header();

        header_builder.set_type_id(4);
        header_builder.set_source("Fred");
    }
    ben(&message);

    println!("Hello");
    std::process::exit(0);
}

fn ben<T>(message: &capnp::message::Builder<T>) -> Result<(), Error>
    where T: capnp::message::Allocator
{
    for _i in 0..std::i64::MAX {
        // 'Send' this message:
        let mut data = Cursor::new(Vec::new());
        {
            serialize::write_message(&mut data, &message);
        }

        let _size = data.position();

        //      'Receive' the message:
        let message_reader = serialize::read_message(
            &mut data,::capnp::message::ReaderOptions::new())?;

        let received_msg = message_reader.get_root::<message4::Reader>()?;

        // Translate the received message to the correct type:
        let mut h = received_msg.get_header()?;
        if h.get_type_id() == 4 {
            //h.set_type_id(16); //TODO
//            h.set_source("Jane");
//            // Use the content of the received msg:
//            h.set_an_int(128);
        }
        data.set_position(0);
    }
    Result::Ok(())
}

#[cfg(test)]
mod tests {
    use test::{Bencher};

    #[bench]
    fn bench_capnproto(b: &mut Bencher) {
        let mut message = ::capnp::message::Builder::new_default();
        {
            let mut a = message.init_root::<super::message4::Builder>();
            a.set_an_int(126);
            a.set_message("This is a variable length string");
            let mut header_builder = a.init_header();

            header_builder.set_type_id(4);
            header_builder.set_source("Fred");
        }

        b.iter(|| {
            // Inner closure, the actual test
           super::ben(&message);
        });
    }
}