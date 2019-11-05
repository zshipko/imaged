use std::cell::RefCell;
use std::collections::BTreeMap;

use gl::types::*;
use glfw::Context as GLFWContext;

pub use glfw::{Action, Key, Modifiers, Scancode, WindowEvent};

#[derive(Debug)]
pub enum Error {
    InvalidColor,
    InvalidBits,
    UnableToCreateWindow,
    Failure,
    Imaged(crate::Error),
    Glfw(glfw::Error),
}

pub struct Context {
    gl: RefCell<glfw::Glfw>,
    windows: BTreeMap<String, Window>,
}

use crate::ffi::ImagedColor::*;

fn image_texture(image: &crate::Image) -> Result<(GLuint, GLuint, GLuint, GLuint), Error> {
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
        crate::ffi::ImagedKind::IMAGED_KIND_INT => match image.meta().bits {
            8 => gl::BYTE,
            16 => gl::SHORT,
            _ => return Err(Error::InvalidBits),
        },
        crate::ffi::ImagedKind::IMAGED_KIND_UINT => match image.meta().bits {
            8 => gl::UNSIGNED_BYTE,
            16 => gl::UNSIGNED_SHORT,
            _ => return Err(Error::InvalidBits),
        },
        crate::ffi::ImagedKind::IMAGED_KIND_FLOAT => match image.meta().bits {
            32 => gl::FLOAT,
            _ => return Err(Error::InvalidBits),
        },
    };

    let internal = match (color, kind) {
        (gl::RED, gl::BYTE) => gl::R8,
        (gl::RED, gl::SHORT) => gl::R16,
        (gl::RED, gl::UNSIGNED_BYTE) => gl::R8UI,
        (gl::RED, gl::UNSIGNED_SHORT) => gl::R16UI,
        (gl::RED, gl::FLOAT) => gl::R32F,
        (gl::RG, gl::BYTE) => gl::RG8,
        (gl::RG, gl::SHORT) => gl::RG16,
        (gl::RG, gl::UNSIGNED_BYTE) => gl::RG8,
        (gl::RG, gl::UNSIGNED_SHORT) => gl::RG16,
        (gl::RG, gl::FLOAT) => gl::RG32F,
        (gl::RGB, gl::BYTE) => gl::RGB8,
        (gl::RGB, gl::SHORT) => gl::RGB16,
        (gl::RGB, gl::UNSIGNED_BYTE) => gl::RGB,
        (gl::RGB, gl::UNSIGNED_SHORT) => gl::RGB16,
        (gl::RGB, gl::FLOAT) => gl::RGB32F,
        (gl::RGBA, gl::BYTE) => gl::RGBA,
        (gl::RGBA, gl::SHORT) => gl::RGBA16,
        (gl::RGBA, gl::UNSIGNED_BYTE) => gl::RGBA,
        (gl::RGBA, gl::UNSIGNED_SHORT) => gl::RGBA16,
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

fn make_window<'a>(app: &Context, key: &str, image: crate::Image) -> Result<Window, Error> {
    let meta = image.meta();
    if let Some((mut display, event)) = app.gl.borrow().create_window(
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
        display.focus();

        return Ok(Window {
            key: key.into(),
            image: RefCell::new(image),
            display,
            event: RefCell::new(event),
            texture_id,
            gl_type,
            gl_color,
            gl_internal,
            framebuffer_id,
        });
    }

    Err(Error::UnableToCreateWindow)
}

impl Context {
    pub fn new() -> Result<Self, Error> {
        let mut gl = glfw::init(glfw::FAIL_ON_ERRORS).unwrap();
        gl.window_hint(glfw::WindowHint::Visible(true));
        Ok(Context {
            gl: RefCell::new(gl),
            windows: BTreeMap::new(),
        })
    }

    pub fn get_window(&self, key: &str) -> Option<&Window> {
        self.windows.get(key)
    }

    pub fn get_window_mut(&mut self, key: &str) -> Option<&mut Window> {
        self.windows.get_mut(key)
    }

    pub fn num_windows(&self) -> usize {
        self.windows.len()
    }

    pub fn create_window(&mut self, key: &str, image: crate::Image) -> Result<(), Error> {
        let window = make_window(self, key, image)?;
        self.windows.insert(key.into(), window);
        Ok(())
    }

    pub fn tick<F: Fn(&mut crate::Image, WindowEvent) -> Result<bool, Error>>(
        &mut self,
        func: F,
    ) -> Result<(), Error> {
        self.gl.borrow_mut().poll_events();
        let mut remove = Vec::new();
        for (key, win) in self.windows.iter_mut() {
            if win.should_close() {
                remove.push(key.to_string());
                continue;
            }

            if !win.is_current() {
                continue;
            }

            {
                let mut event = win.event.borrow_mut();
                let f = glfw::flush_messages(&mut event);
                for (_, event) in f {
                    if !func(&mut win.image.borrow_mut(), event)? {
                        win.display.set_should_close(true)
                    }
                }
            }

            win.draw()?;
        }

        for r in remove {
            self.windows.remove(&r);
        }

        ::std::thread::sleep(std::time::Duration::new(0, 1_000_000_000u32 / 60));
        Ok(())
    }

    pub fn run<F: Fn(&mut crate::Image, WindowEvent) -> Result<bool, Error>>(
        mut self,
        callback: F,
    ) -> Result<(), Error> {
        loop {
            self.tick(|image, event| match event {
                WindowEvent::Key(Key::Escape, _, _, _) => Ok(false),
                _ => callback(image, event),
            })?;

            if self.num_windows() == 0 {
                break;
            }
        }

        Ok(())
    }
}

pub fn empty_callback(_: &mut crate::Image, _: WindowEvent) -> Result<bool, Error> {
    Ok(true)
}

pub struct Window {
    pub key: String,
    pub image: RefCell<crate::Image>,
    framebuffer_id: GLuint,
    texture_id: GLuint,
    gl_color: GLuint,
    gl_internal: GLuint,
    gl_type: GLuint,
    display: glfw::Window,
    event: RefCell<std::sync::mpsc::Receiver<(f64, glfw::WindowEvent)>>,
}

impl Window {
    pub fn should_close(&self) -> bool {
        self.display.should_close()
    }

    pub fn is_current(&self) -> bool {
        self.display.is_focused()
    }

    pub fn draw(&mut self) -> Result<(), Error> {
        self.display.make_current();
        let mut ctx = self.display.render_context();

        gl::load_with(|symbol| self.display.get_proc_address(symbol));

        let meta = self.image.borrow().meta().clone();
        unsafe {
            gl::ClearColor(0.0, 0.0, 0.0, 1.0);
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
