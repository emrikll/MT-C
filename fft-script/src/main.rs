#![feature(iter_next_chunk)]

use std::{env, error::Error, fmt::Write, io::{BufRead, BufReader}, process::exit};
use indicatif::{ProgressBar, ProgressState, ProgressStyle};
use plotters::prelude::*;

use realfft::RealFftPlanner;
use rustfft::num_complex::Complex;
use realfft::num_traits::Pow;

use std::str;
const UPPER_LIMIT: f64 = 0.250;
const LOWER_LIMIT: f64 = 0.220;
const INTERVAL: f64 = 0.000064;
const HERTZ: f64 = 1.0/INTERVAL;
const LINES: usize = 10_000;
const EXPECTED_FREQ: f32 = 1.0 / 0.004048;
const GRAPH_NAME: &str = "graph.png";

fn add_iterator_to_csv_result<T>(pb: &ProgressBar, result_time_vec: &mut Vec<f64>, result_value_vec: &mut Vec<f64>, iter: &[Result<String, T>]){

        for line in iter {
        pb.inc(1);
            if let Ok(string) = line {
                let split_line: Vec<&str> = string.split(',').collect();
                result_value_vec.push(split_line[1].parse().unwrap());
                result_time_vec.push(split_line[0].parse().unwrap());
            }
        }
        

}


fn parse_csv(filename: &String) -> Result<(Vec<f64>,Vec<f64>), Box<dyn Error>> {
    let mut file = std::fs::File::open(filename.clone()).unwrap();
    println!("Indexing file...");
    let mut buf = BufReader::new(&file);
    let count = buf.lines().count();
    
    file = std::fs::File::open(filename).unwrap();

    buf = BufReader::new(&file);

    let mut lines = buf.lines();
    
    let pb = ProgressBar::new(count as u64);
    pb.set_style(ProgressStyle::with_template("{spinner:.green} [{elapsed_precise}] [{wide_bar:.cyan/blue}] {pos}/{len} ({eta})")
        .unwrap()
        .with_key("eta", |state: &ProgressState, w: &mut dyn Write| write!(w, "{:.1}s", state.eta().as_secs_f64()).unwrap())
        .progress_chars("#>-"));
    lines.next();
    let mut result_value_vec: Vec<f64> = vec![];
    let mut result_time_vec: Vec<f64> = vec![];

    loop {
        let chunk = lines.next_chunk::<LINES>();
        if let Err(last) = chunk {
            add_iterator_to_csv_result(&pb, &mut result_time_vec, &mut result_value_vec, last.as_slice());
            break
        }

        let ok_chunk = chunk.unwrap();
        let slice_chunk = ok_chunk.as_slice();
        add_iterator_to_csv_result(&pb, &mut result_time_vec, &mut result_value_vec, slice_chunk);        

    }
         
    Ok((result_value_vec, result_time_vec))
}

fn make_fft(mut value_vec: Vec<f64>) -> Vec<Complex<f64>> {
    // make a planner
    let mut real_planner = RealFftPlanner::<f64>::new();
    // create a FFT
    let r2c = real_planner.plan_fft_forward(value_vec.len());
    // make a dummy real-valued signal (filled with zeros)
    let mut indata = r2c.make_input_vec();
    // make a vector for storing the spectrum
    let mut spectrum = r2c.make_output_vec();

    // forward transform the signal
    let mut test = value_vec.as_mut_slice();
    r2c.process(&mut test, &mut spectrum).unwrap();

    return spectrum

}


fn main() {
        let (mut a, mut b) = parse_csv(&"data/c_0/analog.csv".to_string()).unwrap();
    //println!("{:?}, {:?}",a,b);
    
    let mut spectrum = make_fft(a.clone());

    //println!("{:?}", spectrum[0].re);

    let mut real = Vec::new();
    let mut img = Vec::new();
    let mut format: Vec<(f32, f32)> = vec![];
    let mut low_x = 0 as f64;
    let mut low_y = 0 as f64;
    let mut high_x = 0 as f64;
    let mut high_y = 0 as f64;
    let freq_resolution = HERTZ/(a.len() as f64);
    
    for (index,complex) in spectrum.clone().into_iter().enumerate() {
        real.push(complex.re);
        img.push(complex.im);
        //format.push((complex.re as f32,complex.im as f32));
        
        let mut mag = (complex.re.pow(2) as f64 + complex.im.pow(2) as f64).sqrt();
        let mut freq = freq_resolution * index as f64; 

        if freq < low_x{
            low_x = freq;
        }
        if freq > high_x {
            high_x = freq;
        }
        if mag < low_y{
            low_y = mag;
        }
        if mag > high_y {
            high_y = mag;
        }
        
        format.push((freq as f32, mag as f32));
    }

    //println!("REAL {:?}", real);
    //println!("IMG {:?}", img);
    //println!("Format {:?}", format);

    let root = BitMapBackend::new(GRAPH_NAME, (1920, 1080)).into_drawing_area();
    root.fill(&WHITE).unwrap();
    let mut chart = ChartBuilder::on(&root)
        .caption("Frequency as a FFT", ("sans-serif", 30).into_font())
        .margin(5)
        .x_label_area_size(50)
        .y_label_area_size(50)
        .build_cartesian_2d(low_x as f32..500 as f64 as f32, low_y as f32..50000 as f32).unwrap();

    chart.configure_mesh().draw().unwrap();

    chart
        .draw_series(LineSeries::new(
            format,
            &RED,
        )).unwrap();

    chart.draw_series(LineSeries::new(
        [(EXPECTED_FREQ, 0.0), (EXPECTED_FREQ, 50000.0)],
        &BLUE
    )).unwrap();

    println!("Done! Saved as {}", GRAPH_NAME);
    root.present().unwrap();
}
