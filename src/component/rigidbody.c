#include <stdlib.h>

#include "component/rigidbody.h"
#include "object.h"

static void Update(Object* o, Context* ctx, void* self)
{
	Rigidbody* rb = (Rigidbody*)self;

	rb->linearVelocity = VECTOR_Multiply(rb->linearVelocity, 1 - rb->linearDrag * ctx->time->deltaTime);
	rb->linearVelocity = VECTOR_Multiply(rb->angularVelocity, 1 - rb->angularDrag * ctx->time->deltaTime);

	o->transform->position = VECTOR_Add(o->transform->position, VECTOR_Multiply(rb->linearVelocity, ctx->time->deltaTime));
	o->transform->rotation = VECTOR_Add(o->transform->rotation, VECTOR_Multiply(rb->angularVelocity, ctx->time->deltaTime));
}

static void Destroy(void* self)
{
	Rigidbody* rb = (Rigidbody*)self;
	free(rb);
}

Rigidbody* RIGIDBODY_New()
{
	Rigidbody* rb = malloc(sizeof(Rigidbody));
	if (rb == NULL) return NULL;

	rb->linearVelocity = VECTOR_Zero();
	rb->angularVelocity = VECTOR_Zero();
	rb->linearDrag = 0.f;
	rb->angularDrag = 0.f;

	rb->parent = ACOMPONENT_New(rb, COMPONENT_RIGIDBODY, &Update, &Destroy);
	return rb;
}

void RIGIDBODY_AddForce(Rigidbody* rb, Vector v)
{
	rb->linearVelocity = VECTOR_Add(rb->linearVelocity, v);
}

void RIGIDBODY_AddTorque(Rigidbody* rb, Vector v)
{
	rb->angularVelocity = VECTOR_Add(rb->angularVelocity, v);
}