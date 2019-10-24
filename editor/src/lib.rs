use std::cell::RefCell;
use std::collections::BTreeMap;
use std::path::Path;

use gl::types::*;
use glfw::Context;

mod conv;

pub use conv::halide_buffer;

#[derive(Debug)]
pub enum Error {
    InvalidColor,
    InvalidBits,
    UnableToCreateWindow,
    Failure,
    Imaged(imaged::Error),
    Glfw(glfw::Error),
}

pub struct App<'a> {
    db: imaged::DB,
    joystick: glfw::Joystick,
    ctx: RefCell<glfw::Glfw>,
    windows: BTreeMap<String, Window<'a>>,
}

use imaged::ffi::ImagedColor::*;

fn image_texture(image: &imaged::Image) -> Result<(GLuint, GLuint, GLuint, GLuint), Error> {
    let mut texture_id: GLuint = 0;

    unsafe {
        gl::GenTextures(1, &mut texture_id);
        gl::BindTexture(gl::TEXTURE_2D, texture_id);
    }

    let color = match image.meta().color {
        IMAGED_COLOR_GRAY => gl::RED,
        IMAGED_COLOR_GRAYA => gl::RG,
        IMAGED_COLOR_RGB => gl::RGB,
        IMAGED_COLOR_RGBA => gl::RGBA,
        _ => return Err(Error::InvalidColor),
    };

    let kind = match image.meta().kind {
        imaged::ffi::ImagedKind::IMAGED_KIND_INT => match image.meta().bits {
            8 => gl::BYTE,
            16 => gl::SHORT,
            32 => gl::INT,
            _ => return Err(Error::InvalidBits),
        },
        imaged::ffi::ImagedKind::IMAGED_KIND_UINT => match image.meta().bits {
            8 => gl::UNSIGNED_BYTE,
            16 => gl::UNSIGNED_SHORT,
            32 => gl::UNSIGNED_INT,
            _ => return Err(Error::InvalidBits),
        },
        imaged::ffi::ImagedKind::IMAGED_KIND_FLOAT => match image.meta().bits {
            32 => gl::FLOAT,
            _ => return Err(Error::InvalidBits),
        },
    };

    let internal = match (color, kind) {
        (gl::RED, gl::BYTE) => gl::R8I,
        (gl::RED, gl::SHORT) => gl::R16I,
        (gl::RED, gl::INT) => gl::R32I,
        (gl::RED, gl::UNSIGNED_BYTE) => gl::R8UI,
        (gl::RED, gl::UNSIGNED_SHORT) => gl::R16UI,
        (gl::RED, gl::UNSIGNED_INT) => gl::R32UI,
        (gl::RED, gl::FLOAT) => gl::R32F,
        (gl::RG, gl::BYTE) => gl::RG8I,
        (gl::RG, gl::SHORT) => gl::RG16I,
        (gl::RG, gl::INT) => gl::RG32I,
        (gl::RG, gl::UNSIGNED_BYTE) => gl::RG8UI,
        (gl::RG, gl::UNSIGNED_SHORT) => gl::RG16UI,
        (gl::RG, gl::UNSIGNED_INT) => gl::RG32UI,
        (gl::RG, gl::FLOAT) => gl::RG32F,
        (gl::RGB, gl::BYTE) => gl::RGB8I,
        (gl::RGB, gl::SHORT) => gl::RGB16I,
        (gl::RGB, gl::INT) => gl::RGB32I,
        (gl::RGB, gl::UNSIGNED_BYTE) => gl::RGB8UI,
        (gl::RGB, gl::UNSIGNED_SHORT) => gl::RGB16UI,
        (gl::RGB, gl::UNSIGNED_INT) => gl::RGB32UI,
        (gl::RGB, gl::FLOAT) => gl::RGB32F,
        (gl::RGBA, gl::BYTE) => gl::RGBA8I,
        (gl::RGBA, gl::SHORT) => gl::RGBA16I,
        (gl::RGBA, gl::INT) => gl::RGBA32I,
        (gl::RGBA, gl::UNSIGNED_BYTE) => gl::RGBA8UI,
        (gl::RGBA, gl::UNSIGNED_SHORT) => gl::RGBA16UI,
        (gl::RGBA, gl::UNSIGNED_INT) => gl::RGBA32UI,
        (gl::RGBA, gl::FLOAT) => gl::RGBA32F,
        _ => return Err(Error::InvalidColor),
    };

    unsafe {
        gl::TexImage2D(
            gl::TEXTURE_2D,
            0,
            internal as i32,
            image.meta().width as i32,
            image.meta().height as i32,
            0,
            color,
            kind,
            image.data_ptr(),
        );
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::NEAREST as i32);
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::NEAREST as i32);
        gl::BindTexture(gl::TEXTURE_2D, 0);
    }
    return Ok((texture_id, internal, kind, color));
}

fn make_window<'a>(app: &App, key: &str, image: imaged::Image<'a>) -> Result<Window<'a>, Error> {
    let meta = image.meta();
    if let Some((mut display, event)) = app.ctx.borrow().create_window(
        meta.width as u32,
        meta.height as u32,
        key,
        glfw::WindowMode::Windowed,
    ) {
        display.make_current();

        gl::load_with(|symbol| display.get_proc_address(symbol));

        let mut framebuffer_id = 0;
        let (texture_id, gl_internal, gl_type, gl_color) = image_texture(&image)?;

        unsafe {
            gl::GenFramebuffers(1, &mut framebuffer_id);
            gl::BindFramebuffer(gl::READ_FRAMEBUFFER, framebuffer_id);
            gl::FramebufferTexture2D(
                gl::READ_FRAMEBUFFER,
                gl::COLOR_ATTACHMENT0,
                gl::TEXTURE_2D,
                texture_id,
                0,
            );
            gl::BindFramebuffer(gl::READ_FRAMEBUFFER, 0);
        }

        display.set_all_polling(true);
        display.set_focus_on_show(true);
        display.show();

        let libs = halide_runtime::Manager::new();
        libs.load("filters/filters.so");

        return Ok(Window {
            key: key.into(),
            image: RefCell::new(image),
            display,
            event,
            texture_id,
            gl_type,
            gl_color,
            gl_internal,
            framebuffer_id,
            libs,
            save: false,
            reset: false,
            callback: None,
        });
    }

    Err(Error::UnableToCreateWindow)
}

impl<'a> App<'a> {
    pub fn new<P: AsRef<Path>>(path: P) -> Result<Self, Error> {
        let imaged = imaged::DB::open(path).map_err(Error::Imaged)?;
        let mut ctx = glfw::init(glfw::FAIL_ON_ERRORS).unwrap();
        ctx.window_hint(glfw::WindowHint::Visible(true));
        let joystick = ctx.get_joystick(glfw::JoystickId::Joystick1);
        Ok(App {
            db: imaged,
            joystick,
            ctx: RefCell::new(ctx),
            windows: BTreeMap::new(),
        })
    }

    pub fn window(&self, key: &str) -> Option<&Window<'a>> {
        self.windows.get(key)
    }

    pub fn window_mut(&mut self, key: &str) -> Option<&mut Window<'a>> {
        self.windows.get_mut(key)
    }

    pub fn open_window(&mut self, key: &str) -> Result<(), Error> {
        let handle = self.db.get(key, false).map_err(Error::Imaged)?;
        let image = handle.image().clone();
        let window = make_window(self, key, image)?;
        self.windows.insert(key.into(), window);
        Ok(())
    }

    pub fn create_window(&mut self, key: &str, image: imaged::Image<'a>) -> Result<(), Error> {
        self.db.set_image(key, &image).map_err(Error::Imaged)?;
        let window = make_window(self, key, image)?;
        self.windows.insert(key.into(), window);
        Ok(())
    }

    pub fn tick(&mut self) -> Result<(), Error> {
        let mut remove = Vec::new();
        for (key, win) in self.windows.iter_mut() {
            if win.should_close() {
                remove.push(key.to_string());
                continue;
            }

            if !win.is_current() {
                continue;
            }

            let axes: Vec<f32> = self
                .joystick
                .get_axes()
                .iter()
                .map(|x| (x + 1.0) / 2.0)
                .collect();
            let buttons = self.joystick.get_buttons();

            let f = glfw::flush_messages(&win.event);
            for (_, event) in f {
                handle_event(win, event)?;
            }

            win.draw(&self.db, &axes, &buttons)?;
        }

        for r in remove {
            self.windows.remove(&r);
        }

        self.ctx.borrow_mut().poll_events();

        ::std::thread::sleep(std::time::Duration::new(0, 1_000_000_000u32 / 60));
        Ok(())
    }

    pub fn run(mut self) -> Result<(), Error> {
        loop {
            self.tick()?;
        }
    }
}

fn handle_event(_win: &Window, _event: glfw::WindowEvent) -> Result<(), Error> {
    println!("EVENT: {:?}", _event);
    Ok(())
}

pub struct Window<'a> {
    key: String,
    image: RefCell<imaged::Image<'a>>,
    framebuffer_id: GLuint,
    texture_id: GLuint,
    gl_color: GLuint,
    gl_internal: GLuint,
    gl_type: GLuint,
    display: glfw::Window,
    event: std::sync::mpsc::Receiver<(f64, glfw::WindowEvent)>,
    libs: halide_runtime::Manager,
    save: bool,
    reset: bool,
    callback: Option<fn(&mut Window) -> Result<(), Error>>,
}

impl<'a> Window<'a> {
    pub fn should_close(&self) -> bool {
        self.display.should_close()
    }

    pub fn is_current(&self) -> bool {
        self.display.is_focused()
    }

    pub fn save(&mut self) {
        self.save = true
    }

    pub fn reset(&mut self) {
        self.reset = true
    }

    pub fn load_filter(&self, path: &str) -> Result<(), Error> {
        self.libs.load(path);
        Ok(())
    }

    pub fn filter<T: Copy>(&self, path: &str, name: &str) -> Option<halide_runtime::Filter<T>> {
        self.libs.filter(path, name)
    }

    fn update_image(&mut self, db: &imaged::DB) -> Result<(), Error> {
        if self.save || self.reset {
            let handle = db.get(&self.key, true).map_err(Error::Imaged)?;
            let src = handle.image();

            if self.reset {
                unsafe {
                    std::ptr::copy_nonoverlapping(
                        src.data_ptr(),
                        self.image.borrow().data_ptr(),
                        src.meta().total_bytes(),
                    );
                }
            } else {
                unsafe {
                    std::ptr::copy_nonoverlapping(
                        self.image.borrow().data_ptr(),
                        src.data_ptr(),
                        src.meta().total_bytes(),
                    );
                }
            }

            self.save = false;
            self.reset = false;
        }

        Ok(())
    }

    pub fn draw_cursor(&mut self, axes: &[f32], buttons: &[i32]) -> Result<(), Error> {
        if axes.len() > 0 {
            let meta = self.image.borrow().meta().clone();
            let size = (axes[2] * meta.width.min(meta.height) as f32 / 4.0) as isize;
            let px = imaged::Pixel::rgb(axes[3], axes[4], axes[5]);
            println!("{},{} - {:?} {:?}", axes[0], axes[1], px.data(), buttons);

            let image = self.image.borrow_mut();

            let w = (axes[0] * meta.width as f32) as isize;
            let h = (axes[1] * meta.height as f32) as isize;

            for y in h - size / 2..h + size / 2 {
                for x in w - size / 2..w + size / 2 {
                    if x < 0 || y < 0 {
                        continue;
                    }
                    image.set_pixel(x as usize, y as usize, &px);
                }
            }
        }

        Ok(())
    }

    pub fn set_callback(&mut self, f: Option<fn(&mut Window) -> Result<(), Error>>) {
        self.callback = f
    }

    pub fn draw(&mut self, db: &imaged::DB, axes: &[f32], buttons: &[i32]) -> Result<(), Error> {
        self.display.make_current();
        let mut ctx = self.display.render_context();

        gl::load_with(|symbol| self.display.get_proc_address(symbol));

        if buttons.len() > 0 {
            self.save = buttons[0] == 1;
            self.reset = buttons[1] == 1;
        }

        let meta = self.image.borrow().meta().clone();
        self.update_image(db)?;
        if let Some(f) = self.callback {
            f(self)?;
        }
        self.draw_cursor(axes, buttons)?;
        unsafe {
            gl::ClearColor(0.3, 0.3, 0.3, 1.0);
            gl::Clear(gl::COLOR_BUFFER_BIT);

            gl::BindTexture(gl::TEXTURE_2D, self.texture_id);
            gl::TexImage2D(
                gl::TEXTURE_2D,
                0,
                self.gl_internal as i32,
                meta.width as i32,
                meta.height as i32,
                0,
                self.gl_color,
                self.gl_type,
                self.image.borrow().data_ptr(),
            );
            gl::BindTexture(gl::TEXTURE_2D, 0);

            let meta = self.image.borrow().meta().clone();

            gl::BindFramebuffer(gl::READ_FRAMEBUFFER, self.framebuffer_id);
            gl::FramebufferTexture2D(
                gl::READ_FRAMEBUFFER,
                gl::COLOR_ATTACHMENT0,
                gl::TEXTURE_2D,
                self.texture_id,
                0,
            );
            gl::BlitFramebuffer(
                0,
                meta.height as i32,
                meta.width as i32,
                0,
                0,
                0,
                meta.width as i32,
                meta.height as i32,
                gl::COLOR_BUFFER_BIT,
                gl::LINEAR,
            );
            gl::BindFramebuffer(gl::READ_FRAMEBUFFER, 0);
        }

        ctx.swap_buffers();
        Ok(())
    }
}
