#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TITLE_MAX 80
#define DESC_MAX  240
#define FILE_MAX  256

typedef struct {
    int id;
    char title[TITLE_MAX];
    char desc[DESC_MAX];
    int priority;    // 1..5 (5 самый высокий)
    int done;        // 0/1
    int y, m, d;     // дедлайн (YYYY-MM-DD), 0 если нет
} Task;

typedef struct {
    Task* items;
    size_t size;
    size_t cap;
    int next_id;
} TaskList;

/* ---------- Утилиты безопасного ввода ---------- */

static void trim_newline(char* s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

static int read_line(char* buf, size_t cap) {
    if (!fgets(buf, (int)cap, stdin)) return 0;
    trim_newline(buf);
    return 1;
}

static int read_int(const char* prompt, int minv, int maxv, int* out) {
    char buf[64];
    for (;;) {
        printf("%s", prompt);
        if (!read_line(buf, sizeof(buf))) return 0;

        char* end = NULL;
        long v = strtol(buf, &end, 10);
        while (end && *end && isspace((unsigned char)*end)) end++;

        if (end && *end != '\0') {
            printf("  Ввод не распознан как число. Повтори.\n");
            continue;
        }
        if (v < minv || v > maxv) {
            printf("  Число должно быть в диапазоне [%d..%d].\n", minv, maxv);
            continue;
        }
        *out = (int)v;
        return 1;
    }
}

static int parse_date_yyyy_mm_dd(const char* s, int* y, int* m, int* d) {
    if (!s || !*s) return 0;
    int yy = 0, mm = 0, dd = 0;
    if (sscanf(s, "%d-%d-%d", &yy, &mm, &dd) != 3) return 0;
    if (yy < 1900 || yy > 3000) return 0;
    if (mm < 1 || mm > 12) return 0;
    if (dd < 1 || dd > 31) return 0; // упрощенно
    *y = yy; *m = mm; *d = dd;
    return 1;
}

static void to_lower_ascii(char* s) {
    for (; s && *s; s++) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static int contains_case_insensitive(const char* haystack, const char* needle) {
    if (!needle || !*needle) return 1;
    if (!haystack) return 0;

    char h[512];
    char n[256];
    strncpy(h, haystack, sizeof(h)-1); h[sizeof(h)-1] = '\0';
    strncpy(n, needle, sizeof(n)-1);   n[sizeof(n)-1] = '\0';
    to_lower_ascii(h);
    to_lower_ascii(n);

    return strstr(h, n) != NULL;
}

/* ---------- Динамический список ---------- */

static void list_init(TaskList* L) {
    L->items = NULL;
    L->size = 0;
    L->cap = 0;
    L->next_id = 1;
}

static void list_free(TaskList* L) {
    free(L->items);
    L->items = NULL;
    L->size = 0;
    L->cap = 0;
}

static int list_reserve(TaskList* L, size_t newcap) {
    if (newcap <= L->cap) return 1;
    Task* p = (Task*)realloc(L->items, newcap * sizeof(Task));
    if (!p) return 0;
    L->items = p;
    L->cap = newcap;
    return 1;
}

static int list_push(TaskList* L, const Task* t) {
    if (L->size == L->cap) {
        size_t nc = (L->cap == 0) ? 8 : (L->cap * 2);
        if (!list_reserve(L, nc)) return 0;
    }
    L->items[L->size++] = *t;
    return 1;
}

static Task* list_find_by_id(TaskList* L, int id) {
    for (size_t i = 0; i < L->size; i++) {
        if (L->items[i].id == id) return &L->items[i];
    }
    return NULL;
}

static int list_remove_by_index(TaskList* L, size_t idx) {
    if (idx >= L->size) return 0;
    for (size_t i = idx + 1; i < L->size; i++) {
        L->items[i-1] = L->items[i];
    }
    L->size--;
    return 1;
}

static int list_remove_by_id(TaskList* L, int id) {
    for (size_t i = 0; i < L->size; i++) {
        if (L->items[i].id == id) {
            return list_remove_by_index(L, i);
        }
    }
    return 0;
}

/* ---------- Печать ---------- */

static void print_task(const Task* t) {
    printf("ID: %d | %s | pri=%d | %s\n",
           t->id,
           t->title,
           t->priority,
           t->done ? "DONE" : "TODO");

    if (t->y > 0) {
        printf("  Deadline: %04d-%02d-%02d\n", t->y, t->m, t->d);
    } else {
        printf("  Deadline: (none)\n");
    }

    printf("  Desc: %s\n", t->desc);
}

static void list_print(const TaskList* L) {
    if (L->size == 0) {
        printf("(Список пуст)\n");
        return;
    }
    printf("Всего задач: %zu\n", L->size);
    printf("------------------------------------------------------------\n");
    for (size_t i = 0; i < L->size; i++) {
        print_task(&L->items[i]);
        printf("------------------------------------------------------------\n");
    }
}

/* ---------- Экранирование для файла ---------- */

static void escape_text(const char* src, char* dst, size_t cap) {
    size_t j = 0;
    for (size_t i = 0; src && src[i] != '\0'; i++) {
        char c = src[i];
        const char* rep = NULL;

        if (c == '\\') rep = "\\\\";
        else if (c == '\n') rep = "\\n";
        else if (c == '\t') rep = "\\t";
        else if (c == '\r') rep = ""; // игнор
        if (rep) {
            for (size_t k = 0; rep[k] != '\0'; k++) {
                if (j + 1 < cap) dst[j++] = rep[k];
            }
        } else {
            if (j + 1 < cap) dst[j++] = c;
        }
    }
    if (cap > 0) dst[j < cap ? j : cap-1] = '\0';
}

static void unescape_text(const char* src, char* dst, size_t cap) {
    size_t j = 0;
    for (size_t i = 0; src && src[i] != '\0'; i++) {
        if (src[i] == '\\') {
            char next = src[i+1];
            if (next == '\\') { if (j + 1 < cap) dst[j++] = '\\'; i++; }
            else if (next == 'n') { if (j + 1 < cap) dst[j++] = '\n'; i++; }
            else if (next == 't') { if (j + 1 < cap) dst[j++] = '\t'; i++; }
            else {
                // неизвестная последовательность — копируем как есть
                if (j + 1 < cap) dst[j++] = src[i];
            }
        } else {
            if (j + 1 < cap) dst[j++] = src[i];
        }
    }
    if (cap > 0) dst[j < cap ? j : cap-1] = '\0';
}

/* ---------- Сохранение / Загрузка ---------- */

static int save_to_file(const TaskList* L, const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) return 0;

    fprintf(f, "NEXT_ID\t%d\n", L->next_id);
    for (size_t i = 0; i < L->size; i++) {
        const Task* t = &L->items[i];
        char et[TITLE_MAX * 2];
        char ed[DESC_MAX * 2];
        escape_text(t->title, et, sizeof(et));
        escape_text(t->desc,  ed, sizeof(ed));

        // TASK <id> <priority> <done> <y> <m> <d> <title> <desc>
        fprintf(f, "TASK\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
                t->id, t->priority, t->done, t->y, t->m, t->d, et, ed);
    }

    fclose(f);
    return 1;
}

static int load_from_file(TaskList* L, const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return 0;

    // Сбрасываем текущее
    L->size = 0;
    L->next_id = 1;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (line[0] == '\0') continue;

        if (strncmp(line, "NEXT_ID\t", 8) == 0) {
            L->next_id = atoi(line + 8);
            if (L->next_id < 1) L->next_id = 1;
            continue;
        }

        if (strncmp(line, "TASK\t", 5) == 0) {
            // Разбор табами
            // TASK  id  priority done y m d title desc
            char* parts[9] = {0};
            int pcount = 0;

            char* s = line;
            while (pcount < 9) {
                parts[pcount++] = s;
                char* tab = strchr(s, '\t');
                if (!tab) break;
                *tab = '\0';
                s = tab + 1;
            }
            if (pcount < 9) continue;

            Task t;
            memset(&t, 0, sizeof(t));
            t.id = atoi(parts[1]);
            t.priority = atoi(parts[2]);
            t.done = atoi(parts[3]);
            t.y = atoi(parts[4]);
            t.m = atoi(parts[5]);
            t.d = atoi(parts[6]);

            char title[TITLE_MAX];
            char desc[DESC_MAX];
            unescape_text(parts[7], title, sizeof(title));
            unescape_text(parts[8], desc, sizeof(desc));
            strncpy(t.title, title, sizeof(t.title)-1);
            strncpy(t.desc, desc, sizeof(t.desc)-1);

            if (t.priority < 1) t.priority = 1;
            if (t.priority > 5) t.priority = 5;
            if (t.done != 0) t.done = 1;

            if (!list_push(L, &t)) {
                fclose(f);
                return 0;
            }
        }
    }

    fclose(f);
    return 1;
}

/* ---------- Сортировка ---------- */

static int date_key(const Task* t) {
    if (t->y <= 0) return 99999999; // без дедлайна — вниз
    return t->y * 10000 + t->m * 100 + t->d;
}

static int cmp_by_priority_desc(const void* a, const void* b) {
    const Task* x = (const Task*)a;
    const Task* y = (const Task*)b;
    // сначала не-выполненные
    if (x->done != y->done) return x->done - y->done;
    // потом priority по убыванию
    if (x->priority != y->priority) return y->priority - x->priority;
    // потом id
    return x->id - y->id;
}

static int cmp_by_deadline_asc(const void* a, const void* b) {
    const Task* x = (const Task*)a;
    const Task* y = (const Task*)b;
    // сначала не-выполненные
    if (x->done != y->done) return x->done - y->done;
    int dx = date_key(x);
    int dy = date_key(y);
    if (dx != dy) return dx - dy;
    return x->id - y->id;
}

/* ---------- Операции меню ---------- */

static void op_add(TaskList* L) {
    Task t;
    memset(&t, 0, sizeof(t));
    t.id = L->next_id++;
    t.done = 0;

    printf("Заголовок (до %d символов): ", TITLE_MAX - 1);
    if (!read_line(t.title, sizeof(t.title))) return;
    if (t.title[0] == '\0') strcpy(t.title, "(без названия)");

    printf("Описание (до %d символов): ", DESC_MAX - 1);
    if (!read_line(t.desc, sizeof(t.desc))) return;

    if (!read_int("Приоритет (1..5): ", 1, 5, &t.priority)) return;

    char buf[64];
    printf("Дедлайн (YYYY-MM-DD) или пусто: ");
    if (!read_line(buf, sizeof(buf))) return;

    if (buf[0] == '\0') {
        t.y = t.m = t.d = 0;
    } else {
        if (!parse_date_yyyy_mm_dd(buf, &t.y, &t.m, &t.d)) {
            printf("  Неверный формат даты. Дедлайн не установлен.\n");
            t.y = t.m = t.d = 0;
        }
    }

    if (!list_push(L, &t)) {
        printf("Ошибка: не удалось добавить (нет памяти).\n");
        return;
    }
    printf("Добавлено. ID=%d\n", t.id);
}

static void op_mark_done(TaskList* L) {
    int id = 0;
    if (!read_int("ID задачи: ", 1, 1000000000, &id)) return;
    Task* t = list_find_by_id(L, id);
    if (!t) {
        printf("Не найдено.\n");
        return;
    }
    t->done = 1;
    printf("Готово: ID=%d отмечена выполненной.\n", id);
}

static void op_delete(TaskList* L) {
    int id = 0;
    if (!read_int("ID задачи для удаления: ", 1, 1000000000, &id)) return;
    if (list_remove_by_id(L, id)) {
        printf("Удалено.\n");
    } else {
        printf("Не найдено.\n");
    }
}

static void op_search(const TaskList* L) {
    char q[128];
    printf("Поиск (подстрока): ");
    if (!read_line(q, sizeof(q))) return;
    if (q[0] == '\0') {
        printf("Пустой запрос.\n");
        return;
    }

    int found = 0;
    for (size_t i = 0; i < L->size; i++) {
        const Task* t = &L->items[i];
        if (contains_case_insensitive(t->title, q) ||
            contains_case_insensitive(t->desc, q)) {
            print_task(t);
            printf("------------------------------------------------------------\n");
            found++;
        }
    }
    if (!found) printf("Ничего не найдено.\n");
}

static void op_sort(TaskList* L) {
    printf("1) По приоритету (убыв.), 2) По дедлайну (возр.): ");
    char buf[16];
    if (!read_line(buf, sizeof(buf))) return;

    if (strcmp(buf, "1") == 0) {
        qsort(L->items, L->size, sizeof(Task), cmp_by_priority_desc);
        printf("Отсортировано по приоритету.\n");
    } else if (strcmp(buf, "2") == 0) {
        qsort(L->items, L->size, sizeof(Task), cmp_by_deadline_asc);
        printf("Отсортировано по дедлайну.\n");
    } else {
        printf("Не понял выбор.\n");
    }
}

static void op_save(const TaskList* L) {
    char path[FILE_MAX];
    printf("Файл для сохранения (например, tasks.txt): ");
    if (!read_line(path, sizeof(path))) return;
    if (path[0] == '\0') {
        printf("Пустое имя файла.\n");
        return;
    }
    if (save_to_file(L, path)) printf("Сохранено в %s\n", path);
    else printf("Ошибка сохранения.\n");
}

static void op_load(TaskList* L) {
    char path[FILE_MAX];
    printf("Файл для загрузки: ");
    if (!read_line(path, sizeof(path))) return;
    if (path[0] == '\0') {
        printf("Пустое имя файла.\n");
        return;
    }
    if (load_from_file(L, path)) printf("Загружено из %s\n", path);
    else printf("Ошибка загрузки.\n");
}

static void print_menu(void) {
    printf("\n==== TODO / Notes (C) ====\n");
    printf("1) Показать список\n");
    printf("2) Добавить задачу\n");
    printf("3) Поиск\n");
    printf("4) Отметить выполненной\n");
    printf("5) Удалить\n");
    printf("6) Сортировка\n");
    printf("7) Сохранить в файл\n");
    printf("8) Загрузить из файла\n");
    printf("0) Выход\n");
}

int main(void) {
    TaskList L;
    list_init(&L);

    for (;;) {
        print_menu();
        char choice[16];
        printf("Выбор: ");
        if (!read_line(choice, sizeof(choice))) break;

        if (strcmp(choice, "1") == 0) list_print(&L);
        else if (strcmp(choice, "2") == 0) op_add(&L);
        else if (strcmp(choice, "3") == 0) op_search(&L);
        else if (strcmp(choice, "4") == 0) op_mark_done(&L);
        else if (strcmp(choice, "5") == 0) op_delete(&L);
        else if (strcmp(choice, "6") == 0) op_sort(&L);
        else if (strcmp(choice, "7") == 0) op_save(&L);
        else if (strcmp(choice, "8") == 0) op_load(&L);
        else if (strcmp(choice, "0") == 0) break;
        else printf("Неизвестная команда.\n");
    }

    list_free(&L);
    printf("Пока.\n");
    return 0;
}