use std::convert::TryFrom;

const WORD_LEN: usize = 5;
const WORD_FILE: &'static str = "/usr/share/dict/words";

type Word = [u8; WORD_LEN];

fn read_words() -> Vec<Word> {
    let words = std::fs::read(&WORD_FILE).unwrap();

    words.split(|&byte| byte == b'\n')
        .flat_map(|chunk| -> Option<Word> {
            if chunk.len() == WORD_LEN {
                Some(*<&Word>::try_from(chunk).unwrap())
            } else {
                None
            }
        })
        .collect()
}

/// Known restrictions on a word
struct Restrictions {
    known_letters: [u8; WORD_LEN], // 0 if that letter isn't known, the letter if known
    must_contain: Vec<u8>,         // Must contain each of these letters
    must_not_contain: Vec<u8>      // Must not have any of these letters
}

fn merge_restrictions(r1: &Restrictions, r2: &Restrictions) -> Restrictions {

    Restrictions {
        known_letters: 
    }
}

fn main() {
    let words = read_words();
}
