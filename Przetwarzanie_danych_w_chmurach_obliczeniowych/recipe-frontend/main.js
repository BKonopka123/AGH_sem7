async function getRecipes(sortBy = 'name', filterBy = '', difficulty = '') {
    try {
        const response = await fetch(`http://localhost:5000/recipes?sortBy=${sortBy}&filterBy=${filterBy}&difficulty=${difficulty}`);
        const data = await response.json();

        console.log(data);

        const recipeListElement = document.getElementById('recipe-list');

        if (data.length === 0) {
            recipeListElement.innerHTML = "<p>Brak znalezionych przepisów</p>";
            return;
        }

        recipeListElement.innerHTML = '';
        data.forEach(recipe => {
            const li = document.createElement('li');
            li.textContent = `${recipe.name}`;
            li.addEventListener('click', () => showRecipeDetail(recipe));
            recipeListElement.appendChild(li);
        });
    } catch (error) {
        console.error("Error fetching recipes:", error);
    }
}

function filterRecipes() {
    const difficulty = document.getElementById('difficulty').value;
    getRecipes('name', '', difficulty);
}

function searchRecipes() {
    const searchInput = document.getElementById('search-input');
    const filter = searchInput.value;
    getRecipes('name', filter);
}


document.addEventListener('DOMContentLoaded', () => {
    getRecipes(); 
});

function showRecipeDetail(recipe) {
    const recipeDetailElement = document.getElementById('recipe-detail');
    document.getElementById('recipe-name').textContent = recipe.name;
    document.getElementById('recipe-time').textContent = recipe.time;
    
    const ingredientsList = document.getElementById('ingredients-list');
    ingredientsList.innerHTML = '';

    if (recipe.ingredients && recipe.ingredients.length > 0) {
        recipe.ingredients.forEach(ingredient => {
            const li = document.createElement('li');
            li.textContent = ingredient;
            ingredientsList.appendChild(li);
        });
    }

    recipeDetailElement.style.display = 'block';
    document.getElementById('recipe-list').style.display = 'none';
}

function backToList() {
    document.getElementById('recipe-detail').style.display = 'none';
    document.getElementById('recipe-list').style.display = 'block';
}

async function addRecipe() {
    const name = document.getElementById('name').value;
    const time = document.getElementById('time').value;
    const difficulty = document.getElementById('difficulty-select').value;

    const selectedIngredients = Array.from(document.querySelectorAll('#ingredients-container input:checked'))
        .map(checkbox => checkbox.value);

    if (!name || !time || selectedIngredients.length === 0) {
        alert("Uzupełnij wszystkie pola i wybierz przynajmniej jeden składnik!");
        return;
    }

    try {
        const response = await fetch('http://localhost:5000/recipes', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                name,
                time,
                difficulty,
                ingredients: selectedIngredients
            })
        });

        const data = await response.json();

        if (!response.ok) {
            alert(data.message);
            return;
        }

        alert(data.message);
        getRecipes(); 
    } catch (error) {
        console.error("Error adding recipe:", error);
        alert("Błąd podczas dodawania przepisu");
    }
}

async function addIngredient() {
    const name = document.getElementById('ingredient-name').value;
    const response = await fetch('http://localhost:5000/ingredients', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ name })
    });

    const data = await response.json();
    alert(data.message);
    loadIngredientsForForm();
}

async function loadIngredientsForForm() {
    const response = await fetch('http://localhost:5000/ingredients');
    const data = await response.json();

    const ingredientsList = document.getElementById('ingredients-container');
    ingredientsList.innerHTML = '';

    data.forEach(ingredient => {
        const label = document.createElement('label');
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.value = ingredient;
        label.appendChild(checkbox);
        label.appendChild(document.createTextNode(` ${ingredient}`));
        ingredientsList.appendChild(label);
    });
}

document.addEventListener('DOMContentLoaded', () => {
    getRecipes();
    loadIngredientsForForm();
});