#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

/// Which stream a line goes to, which is the difference between a command that
/// can be piped and one that cannot.
///
/// The convention is one sentence long: **a result goes to stdout, and
/// everything else goes to stderr with a `#` in front of it.** Progress, pool
/// sizes, generator counts, warnings, what was written where, why a search gave
/// up: all commentary, all stderr, all commented.
///
/// The `#` is not decoration. It is the comment character of the SMS format and
/// of the readers in PLinOpt, so a run whose commentary is commented can have
/// its stdout redirected straight into a file that `PMchecker` or `optimizer`
/// reads. Reading what the LJK's own tools read and writing what they read is
/// the point of this layer, and this is the half of it that costs one character.
///
/// Today only `minimise-rank` splits the streams at all, and it writes its
/// commentary bare, so merging the two streams (`2>&1`, which is what a CI log
/// does) produces a file no reader can parse: it stops at the first line that
/// does not look like data, and a bare progress line looks exactly like data.
namespace cli {

/// One piece of commentary, on stderr, with `#` in front of every line of it.
///
/// Built up and written at the end of the statement rather than straight
/// through, because the prefix has to survive a message containing its own
/// newlines: the commands print two-line explanations of why they stopped, and
/// half a commented message is worse than none, since a reader stops at the
/// first line that parses as data rather than as a comment.
class Note {
   public:
    Note() = default;
    Note(const Note&) = delete;
    Note& operator=(const Note&) = delete;

    ~Note() {
        const std::string written = text_.str();
        if (written.empty()) {
            std::cerr << "#\n";
            return;
        }
        std::istringstream lines(written);
        std::string line;
        while (std::getline(lines, line)) {
            std::cerr << (line.empty() ? "#" : "# " + line) << "\n";
        }
    }

    template <class Written>
    Note& operator<<(const Written& value) {
        text_ << value;
        return *this;
    }

   private:
    std::ostringstream text_;
};

/// Commentary: `cli::note() << "pool: " << pool.size();` and the newline follows
/// from the end of the statement. A trailing newline written by hand costs
/// nothing, so a line moved here from a `std::cerr` needs no editing.
inline Note note() { return Note(); }

/// The result, which is what the next program in the pipe reads. Nothing that is
/// not a result may be written here, and that is the whole rule.
inline std::ostream& result() { return std::cout; }

}  // namespace cli
