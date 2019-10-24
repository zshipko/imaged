use imaged_editor::*;

pub fn main() -> Result<(), Error> {
    let args: Vec<String> = std::env::args().skip(1).collect();
    if args.len() < 2 {
        println!("USAGE: imaged-editor [ROOT] [KEY]");
        return Ok(());
    }

    let mut app = App::new(&args[0])?;

    app.open_window(&args[1])?;

    if args.len() >= 4 {
        let win = app.window(&args[1]).unwrap();
        win.load_filter(&args[2])?;
        let filter =
            win.filter::<unsafe extern "C" fn(
                *const halide_runtime::Buffer,
                *mut halide_runtime::Buffer,
            ) -> i32>(&args[2], &args[3]);

        if let Some(filter) = filter {
            let a = halide_buffer(&win.image.borrow());
            let tmp = win.image.borrow().clone();
            let mut b = halide_buffer(&tmp);
            unsafe {
                (filter.get())(&a, &mut b);
            }

            app.create_window("tmp", tmp)?;
        }
    }

    app.run()
}
