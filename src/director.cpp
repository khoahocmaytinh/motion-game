#include "rubymotion.h"
#include "motion-game.h"

/// @class Director < Object
/// Director is a shared object that takes care of the scene graph.

VALUE rb_cDirector = Qnil;
static VALUE mc_director_instance = Qnil;
static std::vector<VALUE> director_using_scene(3);

/// @group Constructors

/// @method .shared
/// @return [Director] the shared Director instance.

static VALUE
director_instance(VALUE rcv, SEL sel)
{
    if (mc_director_instance == Qnil) {
	VALUE obj = rb_cocos2d_object_new(cocos2d::Director::getInstance(), rb_cDirector);
	mc_director_instance = rb_retain(obj);
    }
    return mc_director_instance;
}

/// @group Managing Scenes

/// @method #run(scene)
/// Runs the given scene object.
/// @param scene [Scene] the scene to run.
/// @return [self] the receiver.

static VALUE
director_run(VALUE rcv, SEL sel, VALUE obj)
{
    director_using_scene[0] = rb_retain(obj);
    DIRECTOR(rcv)->runWithScene(rb_any_to_scene(obj));
    return rcv;
}

/// @method #replace(scene)
/// Replaces the current scene with a new one. The running scene will be
/// terminated.
/// @param scene [Scene] the scene to replace the current one with.
/// @return [self] the receiver.

static VALUE
director_replace(VALUE rcv, SEL sel, VALUE obj)
{
    rb_release(director_using_scene[0]);
    director_using_scene[0] = rb_retain(obj);
    DIRECTOR(rcv)->replaceScene(rb_any_to_scene(obj));
    return rcv;
}

/// @method #push(scene)
/// Suspends the execution of the running scene, and starts running the given
/// scene instead.
/// @param scene [Scene] the new scene to run.
/// @return [self] the receiver.

static VALUE
director_push(VALUE rcv, SEL sel, VALUE obj)
{
    director_using_scene.push_back(rb_retain(obj));
    DIRECTOR(rcv)->pushScene(rb_any_to_scene(obj));
    return rcv;
}

/// @method #pop
/// Pops the running scene from the stack, and starts running the previous
/// scene. If there are no more scenes to run, the execution will be stopped.
/// @return [self] the receiver.

static VALUE
director_pop(VALUE rcv, SEL sel)
{
    VALUE last_scene = director_using_scene.back();
    director_using_scene.pop_back();
    rb_release(last_scene);
    DIRECTOR(rcv)->popScene();
    return rcv;
}

/// @method #end
/// Ends the execution of the running scene.
/// @return [self] the receiver.

static VALUE
director_end(VALUE rcv, SEL sel)
{
    for (auto iter = director_using_scene.begin();
	 iter != director_using_scene.end(); ++iter) {
	if (*iter != 0) {
	    rb_release(*iter);
	    *iter = 0;
	}
    }
    DIRECTOR(rcv)->end();
    return rcv;
}

/// @method #pause
/// Pauses the execution of the running scene.
/// @return [self] the receiver.

static VALUE
director_pause(VALUE rcv, SEL sel)
{
    DIRECTOR(rcv)->pause();
    return rcv;
}

/// @method #resume
/// Resumes the execution of the current paused scene.
/// @return [self] the receiver.

static VALUE
director_resume(VALUE rcv, SEL sel)
{
    DIRECTOR(rcv)->resume();
    return rcv;
}

/// @method #start_animation
/// The main loop is triggered again.
/// @return [self] the receiver.

static VALUE
director_start_animation(VALUE rcv, SEL sel)
{
    DIRECTOR(rcv)->startAnimation();
    return rcv;
}

/// @method #stop_animation
/// Stops the animation.
/// @return [self] the receiver.

static VALUE
director_stop_animation(VALUE rcv, SEL sel)
{
    DIRECTOR(rcv)->stopAnimation();
    return rcv;
}

/// @endgroup

/// @property-readonly #origin
/// @return [Point] the visible origin of the director view in points.

static VALUE
director_origin(VALUE rcv, SEL sel)
{
    return rb_ccvec2_to_obj(DIRECTOR(rcv)->getVisibleOrigin());
}

/// @property-readonly #size
/// @return [Size] the visible size of the director view in points.

static VALUE
director_size(VALUE rcv, SEL sel)
{
    return rb_ccsize_to_obj(DIRECTOR(rcv)->getVisibleSize());
}

/// @method #show_stats=(value)
/// @param value [Boolean] true if display the FPS label.
/// Controls whether the FPS (frame-per-second) statistic label is displayed
/// in the bottom-left corner of the director view. By default it is hidden.

static VALUE
director_show_stats_set(VALUE rcv, SEL sel, VALUE val)
{
    DIRECTOR(rcv)->setDisplayStats(RTEST(val));
    return val;
}

/// @method #show_stats?
/// Controls whether the FPS (frame-per-second) statistic label is displayed
/// in the bottom-left corner of the director view. By default it is hidden.
/// @return [Boolean] whether the FPS label is displayed.

static VALUE
director_show_stats(VALUE rcv, SEL sel)
{
    return DIRECTOR(rcv)->isDisplayStats() ? Qtrue : Qfalse;
}

/// @property #content_scale_factor
/// @return [Float] the scale factor of content for multi-resolution.

static VALUE
director_content_scale_factor(VALUE rcv, SEL sel)
{
    return DBL2NUM(DIRECTOR(rcv)->getContentScaleFactor());
}

static VALUE
director_content_scale_factor_set(VALUE rcv, SEL sel, VALUE scale)
{
    DIRECTOR(rcv)->setContentScaleFactor(NUM2DBL(scale));
    return scale;
}

/// @property-readonly #glview
/// @return [GLView] a GLView instance.

static VALUE
director_glview(VALUE rcv, SEL sel)
{
    auto glview = DIRECTOR(rcv)->getOpenGLView();
    return rb_cocos2d_object_new(glview, rb_cGLView);
}

// Internal
#if CC_TARGET_OS_IPHONE || CC_TARGET_OS_APPLETV
static VALUE
director_view_set(VALUE rcv, SEL sel, VALUE obj)
{
    cocos2d::GLView *glview =
	cocos2d::GLViewImpl::createWithEAGLView((void *)obj);
    DIRECTOR(rcv)->setOpenGLView(glview);
    return obj;
}

static VALUE
director_view_get(VALUE rcv, SEL sel)
{
    return (VALUE)DIRECTOR(rcv)->getOpenGLView()->getEAGLView();
}
#endif

extern "C"
void
Init_Director(void)
{
    rb_cDirector = rb_define_class_under(rb_mMC, "Director", rb_cObject);

    rb_define_singleton_method(rb_cDirector, "shared", director_instance, 0);
    rb_define_method(rb_cDirector, "run", director_run, 1);
    rb_define_method(rb_cDirector, "replace", director_replace, 1);
    rb_define_method(rb_cDirector, "push", director_push, 1);
    rb_define_method(rb_cDirector, "pop", director_pop, 0);
    rb_define_method(rb_cDirector, "end", director_end, 0);
    rb_define_method(rb_cDirector, "pause", director_pause, 0);
    rb_define_method(rb_cDirector, "resume", director_resume, 0);
    rb_define_method(rb_cDirector, "start_animation", director_start_animation, 0);
    rb_define_method(rb_cDirector, "stop_animation", director_stop_animation, 0);
    rb_define_method(rb_cDirector, "origin", director_origin, 0);
    rb_define_method(rb_cDirector, "size", director_size, 0);
    rb_define_method(rb_cDirector, "show_stats=", director_show_stats_set, 1);
    rb_define_method(rb_cDirector, "show_stats?", director_show_stats, 0);
    rb_define_method(rb_cDirector, "content_scale_factor", director_content_scale_factor, 0);
    rb_define_method(rb_cDirector, "content_scale_factor=", director_content_scale_factor_set, 1);
    rb_define_method(rb_cDirector, "glview", director_glview, 0);

    // Internal.
#if CC_TARGET_OS_IPHONE || CC_TARGET_OS_APPLETV
    rb_define_method(rb_cDirector, "_set_glview", director_view_set, 1);
    rb_define_method(rb_cDirector, "_get_glview", director_view_get, 0);
#endif
}
