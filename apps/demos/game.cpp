/**
 *  Copyright 2015 Mike Reed
 */

#include "GWindow.h"
#include "../GTime.h"
#include "../../include/GBitmap.h"
#include "../../include/GCanvas.h"
#include "../../include/GColor.h"
#include "../../include/GRandom.h"
#include "../../include/GRect.h"

#include <optional>
#include <vector>

constexpr float kPaddleSpeed = 300;
constexpr float kBallSpeed = 150;

extern float DrawStr(GCanvas*, GPoint origin, const std::string&, float size, const GColor&);

#ifndef NDEBUG
constexpr float kNearlyEqualTolerance = 0.0001f;

static bool nearly_eq(float a, float b, float tol = kNearlyEqualTolerance) {
    assert(tol >= 0);
    return std::abs(a - b) <= tol;
}
#endif

static float dot(GVector a, GVector b) {
    return a.x * b.x + a.y * b.y;
}

static float cross(GVector a, GVector b) {
    return a.x * b.y - a.y * b.x;
}

static GVector normalize(GVector v) {
    const float len = v.length();
    return {v.x/len, v.y/len};
}

static GVector reflect(GVector u, GVector norm) {
    u = {-u.x, -u.y};
    auto c = dot(u, norm),
         s = cross(u, norm);
    return {
        dot({c, -s}, norm),
        dot({s, c}, norm)
    };
}

static GVector rotate_cw(GVector c) { return {c.y, -c.x}; }

struct PointT {
    GPoint m_pt;
    float m_t;
};

struct Ray {
    GPoint  m_origin;
    GVector m_direction;
    float   m_length;

    GVector vec() const { return m_direction * m_length; }
    GPoint start() const { return m_origin; }
    GPoint end() const { return m_origin + this->vec(); }
};

// return the t values for where the two rays cross [ut, vt]
static std::pair<double, double> ray_cross(GPoint a, GVector u, GPoint b, GVector v) {
    auto ab = a - b;
    auto s = (double)cross(v, ab) / cross(u, v);
    double t;

    if (std::abs(v.x) > std::abs(v.y)) {
        t = (ab.x + u.x * s) / v.x;
    } else {
        t = (ab.y + u.y * s) / v.y;
    }
    return {s, t};
}

// If the ray_start is on the CW side of the edge, and its ray_v crosses the edge, this
// returns where it crosses (Point) and the % of ray_v that extends beyond the edge.
// That is: if ray_start + ray_v exactly hits the edge, it will return 0% for the remaining
// ray_v.
static std::optional<PointT> bounce_cw_edge(GPoint edge_start, GVector edge_v, GPoint ray_start, GVector ray_v) {
    auto cr = cross(edge_v, (ray_start - edge_start));
    if (cr <= 0) {   // wrong side
        return {};
    }
    auto [et, rt] = ray_cross(edge_start, edge_v, ray_start, ray_v);
    if (rt <= 0 || rt > 1) {
        return {};  // no intersection
    }
    if (et < 0 || et > 1) {
        return {};  // intersected outside of the finite length of the edge
    }

    // these two should be equal...
    GPoint ep = edge_start + edge_v * et;

    // clean them up for H and V edges (common case)
    if (edge_v.x == 0) {        // vertical edge
        assert(nearly_eq(ep.x, edge_start.x));
        ep.x = edge_start.x;
    } else if (edge_v.y == 0) { // horizontal edge
        assert(nearly_eq(ep.y, edge_start.y));
        ep.y = edge_start.y;
    }

    return {{ep, static_cast<float>(1 - et)}};
}

static std::optional<Ray> reflect_cw_edge(Ray ray, GPoint edge_start, GVector edge_v) {
    if (auto pt = bounce_cw_edge(edge_start, edge_v, ray.m_origin, ray.m_direction * ray.m_length)) {
        auto norm = normalize(rotate_cw(edge_v));
        auto newDir = reflect(ray.m_direction, norm);
        auto newLen = ray.m_length * pt->m_t;

        return {{pt->m_pt, newDir, newLen}};
    }
    return {};
}

static std::optional<Ray> bounce(Ray ray, GRect r, bool insideOut = true) {
    const auto w = r.width(),
               h = r.height();
    std::pair<GPoint, GVector> corners[] = {
        {r.TL(), {w, 0}},
        {r.TR(), {0, h}},
        {r.BR(), {-w, 0}},
        {r.BL(), {0, -h}}
    };
    if (!insideOut) {
        // basically create our edges to be CCW
        corners[0].second = {0, h};
        corners[1].second = {-w, 0};
        corners[2].second = {0, -h};
        corners[3].second = {w, 0};
    }
    for (auto c : corners) {
        if (auto nr = reflect_cw_edge(ray, c.first, c.second)) {
            return *nr;
        }
    }
    return {};
}

struct Paddle {
    GRect m_bounds;
    GColor m_color;
    float m_width;

    void init(GRect bounds, GColor c, float width) {
        m_bounds = bounds;
        m_color = c;
        m_width = width;
    }

    void draw(GCanvas* canvas, GVector offset) {
        auto r = m_bounds;
        // clip
        r.left = std::max(0.f, r.left);
        r.right = std::min(r.right, m_width);
        canvas->fillRect(r.offset(offset), m_color);
    }
};

struct Brick {
    GRect m_bounds;
    GColor m_color;
    int m_scoreValue;

    void draw(GCanvas* canvas, GVector offset) {
        canvas->fillRect(m_bounds.offset(offset).inset(1, 1), m_color);
    }

    std::optional<Ray> collide(Ray ray) const {
        return bounce(ray, m_bounds, false);
    }

    int score() const { return m_scoreValue; }
};

struct Ball {
    GPoint m_pos;
    GVector m_dir;
    float m_speed;
    GColor m_color;

    void init(GPoint pos, GVector dir, float speed, GColor c) {
        m_pos = pos;
        m_dir = normalize(dir);
        m_speed = speed;
        m_color = c;
    }

    void draw(GCanvas* canvas, const GRect& bounds) {
        const float rad = 4;
        GRect r = {m_pos.x - rad, m_pos.y - rad, m_pos.x + rad, m_pos.y + rad};
        canvas->fillRect(r.offset(bounds.TL()), m_color);
    }
};

struct Board {
    GRect m_bounds;
    std::vector<Brick> m_bricks;
    Paddle m_paddle;
    Ball m_ball;
    GSecs m_now = 0;
    int m_score;

    enum class State {
        paused, playing,
    };
    State m_state = State::paused;

    void init(GRect bounds) {
        m_score = 0;
        m_bounds = bounds;
        const size_t W = 14;
        const GColor orange = {1, 0.5f, 0, 1};
        const GColor colors[] = {
                GColor_red, GColor_red, orange, orange,
                GColor_green, GColor_green, GColor_yellow, GColor_yellow,
        };
        const size_t H = std::size(colors);

        const float h = 10, w = m_bounds.width() / W;
        const float gap = m_bounds.top + 5 * h;
        GRect r = { 0, gap, w, gap + h };
        for (size_t y = 0; y < H; ++y) {
            int score = (H - y - 1) | 1;
            for (size_t x = 0; x < W; ++x) {
                m_bricks.push_back({r, colors[y], score});
                r = r.offset(w, 0);
            }
            r = r.offset(-r.left, h);
        }

        // paddle setup
        GColor paddle_color = {0.375f, 0.375f, 1, 1};
        r.bottom = m_bounds.height() - 2 * h;
        r.top = r.bottom - h;
        float paddle_width = w * 2; // narrower for harder levels
        r.left = m_bounds.width()/2 - paddle_width/2;
        r.right = r.left + paddle_width;
        m_paddle.init(r, paddle_color, m_bounds.width());

        m_ball.init({r.center().x, r.top - 10 * h}, {0.7342641f, 1}, kBallSpeed, GColor_white);
    }

    void draw(GCanvas* canvas) {
#if 0
        static GSecs now = 0;
        static int counter = 0;
        if ((counter++ % 60) == 0) {
            double curr = GTime::Secs();
            printf("duration %d\n", (int) ((curr - now) * 1000 / 60));
            now = curr;
        }
#endif

        canvas->fillRect(m_bounds, GColor_black);
        for (auto b : m_bricks) {
            b.draw(canvas, m_bounds.TL());
        }
        m_paddle.draw(canvas, m_bounds.TL());
        m_ball.draw(canvas, m_bounds);
        
        this->drawScore(canvas);
    }

    void movePaddle(float dt) {
        const float speed = kPaddleSpeed;
        auto cx = m_paddle.m_bounds.center().x;
        auto nx = std::clamp(cx + speed * dt, 0.f, m_bounds.width());
        m_paddle.m_bounds = m_paddle.m_bounds.offset(nx - cx, 0);
    }

    enum class PaddleMove {
        none,
        left,
        right
    };
    void tick(PaddleMove move) {
        constexpr float min_tick_time = 0.000001f,
                        max_tick_time = 1;

        const auto now = GTime::Secs();
        if (m_now == 0) {
            m_now = now - 1/30.0;   // not sure how to get started
        }
        float dt = static_cast<float>(now - m_now);
        m_now = now;
        // sanity check, in case we've been in a debugger or some other pause
        assert(dt >= 0);
        dt = std::clamp(dt, min_tick_time, max_tick_time);

        if (move != PaddleMove::none) {
            this->movePaddle(dt * (move == PaddleMove::left ? -1 : 1));
        }

        if (m_state == State::playing) {
            GRect limits = GRect::WH(m_bounds.width(), m_bounds.height());
            Ray ray = {m_ball.m_pos, m_ball.m_dir, m_ball.m_speed * dt};
            bool bounced = false;

            // continue until we exhaust ray's length, or we don't hit anything

            for (;;) {
                bool hit = false;
                for (size_t i = 0; i < m_bricks.size(); ++i) {
                    if (auto nr = m_bricks[i].collide(ray)) {
                        m_score += m_bricks[i].score();
                        ray = *nr;
                        m_bricks.erase(m_bricks.begin() + i);
                        hit = true;
                        bounced = true;
                        break;
                    }
                }
                if (!hit || ray.m_length == 0) {
                    break;
                }
            }

            if (auto nr = reflect_cw_edge(ray, m_paddle.m_bounds.TR(), {-m_paddle.m_bounds.width(), 0})) {
                ray = *nr;
                bounced = true;
            }

            while (auto nr = bounce(ray, limits)) {
                ray = *nr;
                bounced = true;
            }

            if (bounced) {
                m_ball.m_dir = ray.m_direction;
                m_ball.m_pos = ray.m_origin + ray.m_direction * ray.m_length;
            } else {
                m_ball.m_pos += m_ball.m_dir * m_ball.m_speed * dt;
            }
        }
    }

    void togglePlayPause() {
        m_state = m_state == State::paused ? State::playing : State::paused;
    }

    void drawScore(GCanvas* canvas) {
        const float size = 36;
        const float widthScale = 4.0f/6;
        const float width = 3 * widthScale * size;
        GPoint origin = {m_bounds.center().x - width/2, m_bounds.top + 50};
        char buffer[10];
        int n = std::snprintf(buffer, std::size(buffer), "%03d", m_score);
        std::string str(buffer, n);
        DrawStr(canvas, origin, str, size, GColor_white);
    }
};

class TestWindow : public GWindow {
    Board   m_board;
public:
    TestWindow(GISize sz) : GWindow(sz) {
        m_board.init({50, 20, 350, 400});
    }

    virtual ~TestWindow() {}
    
protected:
    void onDraw(GCanvas* canvas) override {

        const auto mask = this->pollFastKeys();
        auto move = Board::PaddleMove::none;
        if (mask & GFastKeys::left_arrow) {
            move = Board::PaddleMove::left;
        } else if (mask & GFastKeys::right_arrow) {
            move = Board::PaddleMove::right;
        }
        m_board.tick(move);

        canvas->clear(GColor_white);
        m_board.draw(canvas);

        this->requestDraw();
    }

    bool onKeyPress(uint32_t sym) override {
        switch (sym) {
            case ' ':
                m_board.togglePlayPause();
                break;
            default:
                break;
        }
        return false;
    }

    GClick* onFindClickHandler(GPoint loc) override {
        return nullptr;
    }

private:
    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new TestWindow({640, 480});

    return wind->run();
}
