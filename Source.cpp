#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <stack>

using namespace std;

class complex_num {

private:

    double re, im;

public:

    complex_num() { re = 0; im = 0; }
    complex_num(double x, double y) { re = x; im = y; }

    double real() {
        return re;
    }

    double imag() {
        return im;
    }

    complex_num add(complex_num b) {

        complex_num c;
        c.re = b.re + this->re;
        c.im = b.im + this->im;
        return c;

    }

    complex_num sub(complex_num b) {

        complex_num c;
        c.re = this->re - b.re;
        c.im = this->im - b.im;
        return c;

    }

    complex_num multiply(complex_num b) {

        complex_num c;
        c.re = this->re * b.re - this->im * b.im;
        c.im = this->re * b.im + this->im * b.im;
        return c;

    }

    complex_num pow2() {

        complex_num c;
        c.re = this->re * this->re - this->im * this->im;
        c.im = 2 * this->re * this->im;

        return c;
    }

    double modulus() {

        return sqrt(this->re * this->re + this->im * this->im);

    }

};

const int IMAGE_WIDTH = 1000;
const int IMAGE_HEIGHT = 600;
const int MAX = 127;

class store2D {
private:

    int front_x, front_y;


public:

    bool* black;
    sf::Color* screen;

    store2D() {

        screen = new sf::Color[600000];
        black = new bool[6000];
        front_x = 0;
        front_y = 0;
        for (int i = 0; i < 6000; i++) {
            black[i] = true;
        }

    }

    int get_frontx() {
        return front_x;
    }

    void change_frontx(int v) {
        front_x = v;
    }

    int get_fronty() {
        return front_y;
    }

    void change_fronty(int v) {
        front_y = v;
    }

    void change(int i, int j, sf::Color col) {
        screen[(front_x + i) % IMAGE_WIDTH + ((front_y + j) % IMAGE_HEIGHT) * IMAGE_WIDTH] = col;
        if (col != sf::Color(0, 0, 0)) {
            black[((front_x + i) % IMAGE_WIDTH) / 10 + (((front_y + j) % IMAGE_HEIGHT) / 10) * IMAGE_WIDTH / 10] = false;
        }
    }

    sf::Color get_col(int i, int j) {
        return screen[(front_x + i) % IMAGE_WIDTH + ((front_y + j) % IMAGE_HEIGHT) * IMAGE_WIDTH];
    }

    bool get_black(int i, int j) {
        return black[((front_x + i) % IMAGE_WIDTH) / 10 + (((front_y + j) % IMAGE_HEIGHT) / 10) * IMAGE_WIDTH / 10];
    }

};


class mandelbrot_zoom {

private:
    double offsetX, offsetY;
    double zoom;
    double zoomfactor;
    store2D drawing;
    int distance;

public:
    bool stateChanged;
    mandelbrot_zoom() {

        //default
        zoom = 0.004;
        zoomfactor = 0.8;
        distance = 40;
        offsetX = -0.7;
        offsetY = 0;

        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                complex_num c((x - double(IMAGE_WIDTH / 2.0)) * zoom + offsetX, (y - double(IMAGE_HEIGHT / 2.0)) * zoom + offsetY);
                int value = compute(c, MAX);
                drawing.change(x, y, get_color(value));
            }
        }

    }

    store2D getdrawing(){
        return drawing;
    }
    
    void setpixel(sf::Color col, int i) {
        drawing.screen[i] = col;
    }

    void setdrawing(store2D temp){
        drawing = temp;
    }

    void setZoom(double z) {
        zoom = z;
    }

    void setZoomFactor(double zf) {
        zoomfactor = zf;
    }

    void setDistance(int d) {
        distance = d;
    }

    int compute(complex_num c, int maximum) {

        int counter = 0;
        complex_num z(c.real(), c.imag());


        while (z.modulus() <= 2.0 && counter <= maximum) {
            complex_num z_next = z.pow2();
            z_next = z_next.add(c);
            z = z_next;
            if (z.real() == c.real() && z.imag() == c.imag()) { // a repetition indicates that the point is in the Mandelbrot set
                return -1; // points in the Mandelbrot set are represented by a return value of -1
            }
            counter += 1;
        }

        if (counter >= maximum) {
            return -1; // -1 is used here to indicate that the point lies within the Mandelbrot set
        }
        else {
            return counter; // returning the number of iterations allows for colouring
        }
    }

    sf::Color get_color(int iterations) {
        int r, g, b;

        if (iterations == -1) {
            r = 0;
            g = 0;
            b = 0;
        }
        else if (iterations == 0) {
            r = 255;
            g = 0;
            b = 0;
        }
        else {
            // colour gradient:      Red -> Blue -> Green -> Red -> Black
            // corresponding values:  0  ->  16  ->  32   -> 64  ->  127 (or -1)
            if (iterations < 16) {
                r = 16 * (16 - iterations);
                g = 0;
                b = 16 * iterations - 1;
            }
            //
            //
            else if (iterations < 32) {
                r = 0;
                g = 16 * (iterations - 16);
                b = 16 * (32 - iterations) - 1;
            }
            else if (iterations < 64) {
                //
                r = 8 * (iterations - 32);
                g = 8 * (64 - iterations) - 1;
                b = 0;
            }
            //
            else { // range is 64 - 127
                r = 255 - (iterations - 64) * 4;
                g = 0;
                b = 0;
            }
        }

        return sf::Color(r, g, b);
    }

    void display() {

        sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Mandelbrot");
        window.setFramerateLimit(30);

        sf::Image image;

        image.create(IMAGE_WIDTH, IMAGE_HEIGHT, sf::Color(0, 0, 0));
        sf::Texture texture;
        sf::Sprite sprite;

        stateChanged = true; // track whether the image needs to be regenerated

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {

                switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:

                    stateChanged = true; // image needs to be recreated when the user changes zoom or offset
                    switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                    case sf::Keyboard::Equal:
                        zoom_in();
                        break;
                    case sf::Keyboard::Dash:                        
                        zoom_out();
                        break;
                    case sf::Keyboard::W:
                        move_up();
                        break;
                    case sf::Keyboard::S:
                        move_down();
                        break;
                    case sf::Keyboard::A:
                        move_left();
                        break;
                    case sf::Keyboard::D:
                        move_right();
                        break;
                    default: break;
                    }
                default:
                    break;
                }
            }



            if (stateChanged) { // only generate a new image if something has changed, to avoid unnecessary lag

                for (int x = 0; x < IMAGE_WIDTH; x++) {
                    for (int y = 0; y < IMAGE_HEIGHT; y++) {
                        image.setPixel(x, y, drawing.get_col(x, y));
                    }
                }

                texture.loadFromImage(image);
                sprite.setTexture(texture);
            }


            window.clear();
            window.draw(sprite);
            window.display();

            stateChanged = false;
        }


    }

    

    void move_left() {
        offsetX -= distance * zoom;

        drawing.change_frontx((drawing.get_frontx() - distance + IMAGE_WIDTH) % IMAGE_WIDTH);
        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < distance; x++) {
                complex_num c((x - double(IMAGE_WIDTH / 2.0)) * zoom + offsetX, (y - double(IMAGE_HEIGHT / 2.0)) * zoom + offsetY);
                int value = compute(c, MAX);
                drawing.change(x, y, get_color(value));
            }
        }

    }

    void move_right() {
        offsetX += distance * zoom;

        drawing.change_frontx((drawing.get_frontx() + distance + IMAGE_WIDTH) % IMAGE_WIDTH);
        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = IMAGE_WIDTH - distance; x < IMAGE_WIDTH; x++) {
                complex_num c((x - double(IMAGE_WIDTH / 2.0)) * zoom + offsetX, (y - double(IMAGE_HEIGHT / 2.0)) * zoom + offsetY);
                int value = compute(c, MAX);
                drawing.change(x, y, get_color(value));
            }
        }

    }

    void move_up() {
        offsetY -= distance * zoom;

        drawing.change_fronty((drawing.get_fronty() - distance + IMAGE_HEIGHT) % IMAGE_HEIGHT);
        for (int y = 0; y < distance; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                complex_num c((x - double(IMAGE_WIDTH / 2.0)) * zoom + offsetX, (y - double(IMAGE_HEIGHT / 2.0)) * zoom + offsetY);
                int value = compute(c, MAX);
                drawing.change(x, y, get_color(value));
            }
        }

    }

    void move_down() {
        offsetY += distance * zoom;

        drawing.change_fronty((drawing.get_fronty() + distance + IMAGE_HEIGHT) % IMAGE_HEIGHT);
        for (int y = IMAGE_HEIGHT - distance; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                complex_num c((x - double(IMAGE_WIDTH / 2.0)) * zoom + offsetX, (y - double(IMAGE_HEIGHT / 2.0)) * zoom + offsetY);
                int value = compute(c, MAX);
                drawing.change(x, y, get_color(value));
            }
        }

    }


    void zoom_in() {
        zoom *= zoomfactor;

        store2D drawing_temp;

        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                double x1 = ((double(x) - double(IMAGE_WIDTH / 2.0)) *zoomfactor) + IMAGE_WIDTH / 2.0;
                double y1 = ((double(y) - double(IMAGE_HEIGHT / 2.0)) * zoomfactor) + IMAGE_HEIGHT / 2.0;

                if ((x1 - IMAGE_WIDTH / 2.0 - floor(x1 - IMAGE_WIDTH / 2.0) == 0) && (y1 - IMAGE_HEIGHT / 2.0 - floor(y1 - IMAGE_HEIGHT / 2.0) == 0)) {
                    drawing_temp.change(x, y, drawing.get_col(int(x1), int(y1)));
                }
                else if (drawing.get_black(int(x1), int(y1)) == true) {
                    drawing_temp.change(x, y, sf::Color(0, 0, 0));
                }
                else {
                    complex_num c((x - double(IMAGE_WIDTH / 2.0)) * zoom + offsetX, (y - double(IMAGE_HEIGHT / 2.0)) * zoom + offsetY);
                    int value = compute(c, MAX);
                    drawing_temp.change(x, y, get_color(value));
                }
            }
        }

        delete drawing.screen;
        delete drawing.black;
        drawing = drawing_temp;
    }


    void zoom_out() {
        zoom /= zoomfactor;
        store2D drawing_temp;

        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                int x1 = ((double(x) - double(IMAGE_WIDTH / 2.0)) / zoomfactor) + IMAGE_WIDTH / 2.0;
                int y1 = ((double(y) - double(IMAGE_HEIGHT / 2.0)) / zoomfactor) + IMAGE_HEIGHT / 2.0;

                if (x1 < IMAGE_WIDTH && x1 >= 0 && y1 < IMAGE_HEIGHT && y1 >= 0) {
                    drawing_temp.change(x, y, drawing.get_col(x1, y1));
                }
                else {
                    complex_num c((x - double(IMAGE_WIDTH / 2.0)) * zoom + offsetX, (y - double(IMAGE_HEIGHT / 2.0)) * zoom + offsetY);
                    int value = compute(c, MAX);
                    drawing_temp.change(x, y, get_color(value));
                }
            }
        }

        delete drawing.screen;
        delete drawing.black;
        drawing = drawing_temp;
    }


};

/*
class stackdrawings {
private:
public:
    store2D* draws;
    int front;
    int capacity;

    stackdrawings() {
        front = -1;
        capacity = 10;
        draws = new store2D[10];
    }

    void push(store2D pic) { 
        if (front >= (capacity-1)) { 
            cout << "No Space"; 
            return;
        } 
        front++; 
        draws[front] = pic;
    }

    store2D top() {
        if (!isEmpty()) {
            return draws[front];
        }
        else cout << "Stack empty!";
    }

    void pop() { 
        if (!isEmpty()) {
            front--;
        }
        else cout << "Stack empty!";
    }

    bool isEmpty() {
        if (front == -1) return true;
        else return false;
    }    

};
*/

class Simulations {

private:
    
    //stack<store2D> StoredDraw;
    stack<sf::Color*> StoredDraw;
    stack<int> fronts_x;
    stack<int> fronts_y;
    //stack<bool*> blacks;
    mandelbrot_zoom M;

public:

    Simulations() {
        M.setDistance(40);
        M.setZoomFactor(0.8);
    }

    Simulations(int distance, double zoomf) {
        M.setDistance(distance);
        M.setZoomFactor(zoomf);
    }

    void start(int i) {
    
            int j = 0;
            
            sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Mandelbrot");
            window.setFramerateLimit(30);

            sf::Image image;

            image.create(IMAGE_WIDTH, IMAGE_HEIGHT, sf::Color(0, 0, 0));
            
            sf::Texture texture;
            sf::Sprite sprite;

            bool stateChanged = true; // track whether the image needs to be regenerated

            while (window.isOpen()) {
                sf::Event event;
                while (window.pollEvent(event)) {

                    switch (event.type) {
                    case sf::Event::Closed:
                        window.close();
                        break;
                    case sf::Event::KeyPressed:

                        stateChanged = true; // image needs to be recreated when the user changes zoom or offset
                        switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;

                        default: break;
                        }

                    case sf::Event::MouseButtonPressed:
                        stateChanged = true;
                        if (i == 0) {    

                            if (j < 11) {
                                sf::Color* temp = new sf::Color[600000];
                                //bool* temp_black = new bool[6000];
                                for (int i = 0; i < 600000; i++) {
                                    //if (i < 6000) { temp_black[i] = M.getdrawing().black[i]; }
                                    temp[i] = M.getdrawing().screen[i];
                                }
                                StoredDraw.push(temp);
                                //blacks.push(temp_black);
                                //StoredDraw.push(M.getdrawing().screen);


                                fronts_x.push(M.getdrawing().get_frontx());
                                fronts_y.push(M.getdrawing().get_fronty());
                                //blacks.push(M.getdrawing().black);

                                if (j % 2 == 0) { M.move_left(); }
                                else { M.zoom_in(); };
                                j++;

                                
                            }
                            else if (j >= 11 && j < 18) {
                                //one way can be using copy operation here after storing the address of the screen and black
                                for (int i = 0; i < 600000; i++) {
                                    //if (i < 6000) { M.getdrawing().black[i]=blacks.top()[i]; }
                                    M.setpixel(StoredDraw.top()[i], i);
                                }
                                //delete blacks.top();
                                delete StoredDraw.top();

                                M.getdrawing().change_frontx(fronts_x.top());
                                M.getdrawing().change_fronty(fronts_y.top());
                                //M.getdrawing().screen=StoredDraw.top();
                                //M.getdrawing().black = blacks.top();
                                StoredDraw.pop();
                                //blacks.pop();
                                fronts_x.pop();
                                fronts_y.pop();
                                j++;
                                //delete TEMP.screen;
                                //delete TEMP.black;
                            }
                            else return;
                        }
                    default:
                        break;
                    }
                }

                if (stateChanged) { // only generate a new image if something has changed, to avoid unnecessary lag

                    for (int x = 0; x < IMAGE_WIDTH; x++) {
                        for (int y = 0; y < IMAGE_HEIGHT; y++) {
                            image.setPixel(x, y, M.getdrawing().get_col(x, y));
                            
                        }
                    }

                    //delete M.getdrawing().screen;
                    //delete M.getdrawing().black;

                    texture.loadFromImage(image);
                    sprite.setTexture(texture);
                }


                window.clear();
                window.draw(sprite);
                window.display();

                stateChanged = false;
            }


    }

       
};


int main() {
    //mandelbrot_zoom M1;
    //M1.display();

    Simulations S;
    S.start(0);
}
