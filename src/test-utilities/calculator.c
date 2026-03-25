/**
 * A simple calculator application using GTK.
 *
 * @author agueguen-LR <agueguen@proton.me>
 * @date 2026
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <stdbool.h>

static GtkWindow *window;
// Variable to hold the current number being input by the user before it's emitted as a token.
static long long current_number = 0;
// Mark to track where the current number starts in the result text buffer for easy updates.
static GtkTextMark* number_start_mark;
// Flag to indicate if the user is currently inputting a number.
static bool number_in_progress = false;
// Text buffers for displaying the current expression/result and any error messages.
static GtkTextBuffer *result_buffer;
static GtkTextBuffer *error;
// Queue to hold the tokens for the current expression.
static GQueue* tokens;

/**
 * Token types for parsing, evaluating and the shunting yard algorithm.
 * The value field in the Token struct is used for both numbers and operator precedence.
 */
typedef enum {
	TOKEN_NUMBER,
	TOKEN_ADD,
	TOKEN_SUBTRACT,
	TOKEN_MULTIPLY,
	TOKEN_DIVIDE,
} TokenType;

/**
 * Token structure to represent numbers and operators.
 * For numbers, the value field holds the actual number.
 * For operators, the value field holds the precedence (1 for + and -, 2 for * and /).
 */
typedef struct {
	TokenType type;
	long long value; // number or operator precedence
} Token;

/**
 * @brief Converts the infix expression represented by the tokens queue into postfix notation using the shunting yard algorithm.
 * Replaces the tokens queue directly.
 */
static void shunting_yard() {
	GQueue* output_queue = g_queue_new();
	GQueue* operator_stack = g_queue_new();

	for (int i = 0; i < tokens->length; i++) {
		Token* token = g_queue_peek_nth(tokens, i);
		if (token->type == TOKEN_NUMBER) {
			g_queue_push_tail(output_queue, token);
		} else {
			while (!g_queue_is_empty(operator_stack)) {
				Token* top_op = g_queue_peek_tail(operator_stack);
				if (top_op->type != TOKEN_NUMBER && top_op->value >= token->value) {
					g_queue_push_tail(output_queue, g_queue_pop_tail(operator_stack));
				} else {
					break;
				}
			}
			g_queue_push_tail(operator_stack, token);
		}
	}

	while (!g_queue_is_empty(operator_stack)) {
		g_queue_push_tail(output_queue, g_queue_pop_tail(operator_stack));
	}

	g_queue_free(tokens);
	g_queue_free(operator_stack);
	tokens = output_queue;
}

/**
 * @brief Handler for when a number button is clicked. Updates the current number being input and the display.
 * Also checks for overflow and updates the error message if the number is too large.
 *
 * @param widget The button that was clicked (not used).
 * @param number The integer value of the button that was clicked (0-9).
 */
static void number_clicked(GtkWidget *widget, gpointer number) {
	number_in_progress = true;
	gtk_text_buffer_set_text(error, "", -1);
	if (current_number > (LLONG_MAX - GPOINTER_TO_INT(number)) / 10) {
		gtk_text_buffer_set_text(error, "Number too large, input ignored", -1);
		return;
	}
	GtkTextIter start_iter, end_iter;
	gtk_text_buffer_get_iter_at_mark(result_buffer, &start_iter, number_start_mark);
	gtk_text_buffer_get_end_iter(result_buffer, &end_iter);
	gtk_text_buffer_delete(result_buffer, &start_iter, &end_iter);

	char buffer_text[20];
  current_number = current_number*10 + GPOINTER_TO_INT(number);
	snprintf(buffer_text, sizeof(buffer_text), "%lld", current_number);
	gtk_text_buffer_insert(result_buffer, &start_iter, buffer_text, -1);
}

/**
 * @brief Emits the current number as a token and resets the current number state.
 * This is called when an operator is clicked or when the equals button is pressed to ensure the last number is included in the expression.
 */
static void emit_number_token() {
	if (number_in_progress) {
		Token *num_token = g_new(Token, 1);
		num_token->type = TOKEN_NUMBER;
		num_token->value = current_number;
		g_queue_push_tail(tokens, num_token);
		current_number = 0;
		number_in_progress = false;
	}
}

/**
 * @brief Handler for when an operator button is clicked. Emits the current number as a token, adds the operator token to the queue, and updates the display.
 *
 * @param widget The button that was clicked (not used).
 * @param operator The integer value representing the operator type (TOKEN_ADD, TOKEN_SUBTRACT, TOKEN_MULTIPLY, TOKEN_DIVIDE).
 */
static void operator_clicked(GtkWidget *widget, gpointer operator) {
	gtk_text_buffer_set_text(error, "", -1);
	if (!number_in_progress) {
		gtk_text_buffer_set_text(error, "Input a number first", -1);
		return;
	}

	emit_number_token();

	GtkTextIter end_iter;
	gtk_text_buffer_get_end_iter(result_buffer, &end_iter);

	Token *op_token = g_new(Token, 1);
	TokenType type = GPOINTER_TO_INT(operator);
	switch (type) {
		case TOKEN_ADD:
			gtk_text_buffer_insert(result_buffer, &end_iter, " + ", -1);
			op_token->type = TOKEN_ADD;
			op_token->value = 1;
			g_queue_push_tail(tokens, op_token);
			break;
		case TOKEN_SUBTRACT:
			gtk_text_buffer_insert(result_buffer, &end_iter, " - ", -1);
			op_token->type = TOKEN_SUBTRACT;
			op_token->value = 1;
			g_queue_push_tail(tokens, op_token);
			break;
		case TOKEN_MULTIPLY:
			gtk_text_buffer_insert(result_buffer, &end_iter, " * ", -1);
			op_token->type = TOKEN_MULTIPLY;
			op_token->value = 2;
			g_queue_push_tail(tokens, op_token);
			break;
		case TOKEN_DIVIDE:
			gtk_text_buffer_insert(result_buffer, &end_iter, " / ", -1);
			op_token->type = TOKEN_DIVIDE;
			op_token->value = 2;
			g_queue_push_tail(tokens, op_token);
			break;
		default:
			gtk_text_buffer_set_text(result_buffer, "Unknown operator", -1);
			return;
	}

	gtk_text_buffer_get_end_iter(result_buffer, &end_iter);
	gtk_text_buffer_move_mark(result_buffer, number_start_mark, &end_iter);
}

/**
 * @brief Handler for when the clear button is clicked. Resets the current number, clears the display and error messages, and empties the tokens queue.
 */
static void clear() {
	current_number = 0;
	gtk_text_buffer_set_text(result_buffer, "", -1);
	GtkTextIter start_iter;
	gtk_text_buffer_get_start_iter(result_buffer, &start_iter);
	g_queue_clear(tokens);
	number_in_progress = false;
}

/**
 * @brief Evaluates a binary operation on the top two numbers on the stack. Checks for overflow and division by zero.
 *
 * @param stack The evaluation stack containing number tokens.
 * @param type The type of the operator to apply (TOKEN_ADD, TOKEN_SUBTRACT, TOKEN_MULTIPLY, TOKEN_DIVIDE).
 * @return true if the operation was successful and the result was pushed back on the stack, false if there was an error (overflow or division by zero).
 */
static bool eval_binary_op(GQueue* stack, TokenType type) {
	Token* b = g_queue_pop_tail(stack);
	Token* a = g_queue_pop_tail(stack);

	long long result;

	switch (type) {
		case TOKEN_ADD:
			if (__builtin_add_overflow(a->value, b->value, &result)) goto overflow;
			break;

		case TOKEN_SUBTRACT:
			if (__builtin_sub_overflow(a->value, b->value, &result)) goto overflow;
			break;

		case TOKEN_MULTIPLY:
			if (__builtin_mul_overflow(a->value, b->value, &result)) goto overflow;
			break;

		case TOKEN_DIVIDE:
			if (b->value == 0) {
				gtk_text_buffer_set_text(error, "Division by zero", -1);
				g_free(a);
				g_free(b);
				return FALSE;
			}
			result = a->value / b->value;
			break;

		default:
			g_free(a);
			g_free(b);
			return FALSE;
	}

	Token* res = g_new(Token, 1);
	res->type = TOKEN_NUMBER;
	res->value = result;

	g_queue_push_tail(stack, res);
	g_free(a);
	g_free(b);
	return TRUE;

overflow:
	gtk_text_buffer_set_text(error, "Number too large", -1);
	g_free(a);
	g_free(b);
	return FALSE;
}

/**
 * @brief Handler for when the equals button is clicked. 
 * Finalizes the current number input, converts the expression to postfix notation,
 * evaluates it, and updates the display with the result or any error messages.
 */
static void equals() {
	if (!number_in_progress) {
		gtk_text_buffer_set_text(error, "Input a number first", -1);
		return;
	}

	emit_number_token();
	gtk_text_buffer_set_text(error, "", -1);

	shunting_yard();

	GQueue* eval_stack = g_queue_new();

	for (int i = 0; i < tokens->length; i++) {
		Token* token = g_queue_peek_nth(tokens, i);

		if (token->type == TOKEN_NUMBER) {
			g_queue_push_tail(eval_stack, token);
			continue;
		}

		if (!eval_binary_op(eval_stack, token->type)) {
			g_queue_free(eval_stack);
			clear();
			return;
		}
		g_free(token);
	}

	if (eval_stack->length != 1) {
		gtk_text_buffer_set_text(error, "Error in evaluation", -1);
		g_queue_free(eval_stack);
		clear();
		return;
	}

	Token* result_token = g_queue_pop_tail(eval_stack);

	clear();
	current_number = result_token->value;
	number_in_progress = true;

	char buffer_text[20];
	snprintf(buffer_text, sizeof(buffer_text), "%lld", result_token->value);
	gtk_text_buffer_set_text(result_buffer, buffer_text, -1);

	g_free(result_token);
	g_queue_free(eval_stack);
}

/**
 * @brief Handler for when the application is activated. 
 * Sets up the main window, the display for the current expression/result, the buttons for input, and the error display.
 *
 * @param app The GtkApplication instance (not used).
 * @param user_data User data passed to the callback (not used).
 */
static void on_activate(GtkApplication *app, gpointer user_data) {
  window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_title(window, "Image Viewer");
  gtk_window_set_default_size(window, 900, 700);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  result_buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(result_buffer, "", -1);
	GtkTextIter start_iter;
	gtk_text_buffer_get_start_iter(result_buffer, &start_iter);
	gtk_text_buffer_create_mark(result_buffer, "number_start", &start_iter, TRUE);
	number_start_mark = gtk_text_buffer_get_mark(result_buffer, "number_start");
  GtkWidget *result_view = gtk_text_view_new_with_buffer(result_buffer);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(result_view), FALSE);

	tokens = g_queue_new();

  GtkWidget *grid = gtk_grid_new();

  // 1-9 buttons
  for (int i = 1; i < 10; i++) {
		char num[2];
		snprintf(num, sizeof(num), "%d", i);
    GtkWidget *button = gtk_button_new_with_label(num);
    g_signal_connect(button, "clicked", G_CALLBACK(number_clicked), GINT_TO_POINTER(i));
    gtk_grid_attach(GTK_GRID(grid), button, (i - 1) % 3, (i - 1) / 3, 1, 1);
  }

  GtkWidget *_0 = gtk_button_new_with_label("0");
  g_signal_connect(_0, "clicked", G_CALLBACK(number_clicked), GINT_TO_POINTER(0));
  GtkWidget *_add = gtk_button_new_with_label("+");
	g_signal_connect(_add, "clicked", G_CALLBACK(operator_clicked), GINT_TO_POINTER(TOKEN_ADD));
  GtkWidget *_subtract = gtk_button_new_with_label("-");
	g_signal_connect(_subtract, "clicked", G_CALLBACK(operator_clicked), GINT_TO_POINTER(TOKEN_SUBTRACT));
  GtkWidget *_multiply = gtk_button_new_with_label("*");
	g_signal_connect(_multiply, "clicked", G_CALLBACK(operator_clicked), GINT_TO_POINTER(TOKEN_MULTIPLY));
  GtkWidget *_divide = gtk_button_new_with_label("/");
	g_signal_connect(_divide, "clicked", G_CALLBACK(operator_clicked), GINT_TO_POINTER(TOKEN_DIVIDE));
  GtkWidget *_clear = gtk_button_new_with_label("C");
	g_signal_connect(_clear, "clicked", G_CALLBACK(clear), NULL);
	GtkWidget *_equals = gtk_button_new_with_label("=");
	g_signal_connect(_equals, "clicked", G_CALLBACK(equals), NULL);

  gtk_grid_attach(GTK_GRID(grid), _clear, 2, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), _0, 1, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), _equals, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), _add, 3, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), _subtract, 3, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), _multiply, 3, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), _divide, 3, 3, 1, 1);

  error = gtk_text_buffer_new(NULL);
  GtkWidget *error_view = gtk_text_view_new_with_buffer(error);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(error_view), FALSE);

  gtk_box_pack_start(GTK_BOX(box), result_view, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), grid, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), error_view, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(window), box);

  gtk_widget_show_all(GTK_WIDGET(window));
}

int main(int argc, char *argv[]) {
	// G_APPLICATION_FLAGS_NONE is deprecated but the system we have to test on
	// is old so it stays
  GtkApplication *app = gtk_application_new("com.example.GtkApplication",
                                            G_APPLICATION_FLAGS_NONE);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
