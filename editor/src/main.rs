use imaged_editor::*;

pub fn main() -> Result<(), Error> {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let mut app = App::new(&args[0])?;
    app.open_window(&args[1])?;
    app.run()
}
