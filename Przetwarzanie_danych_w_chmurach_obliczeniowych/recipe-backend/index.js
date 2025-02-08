const express = require('express');
const cors = require('cors');
const neo4j = require('neo4j-driver');

const app = express();
const port = 5000;

app.use(cors());
app.use(express.json());

const driver = neo4j.driver(
  process.env.NEO4J_URI,
  neo4j.auth.basic(process.env.NEO4J_USER, process.env.NEO4J_PASSWORD)
);
const session = driver.session();

app.get('/recipes', async (req, res) => {
    const { sortBy, filterBy, difficulty } = req.query;
    const session = driver.session();

    let query = 'MATCH (r:Recipe)-[:USES]->(i:Ingredient)';

    if (filterBy) {
        query += ` WHERE r.name CONTAINS '${filterBy}' OR i.name CONTAINS '${filterBy}'`;
    }

    if (difficulty) {
        query += ` WHERE r.difficulty = '${difficulty}'`;
    }

    if (sortBy) {
        query += ` ORDER BY r.${sortBy}`;
    }

    query += ' RETURN r.name AS name, r.time AS time, r.difficulty AS difficulty, collect(i.name) AS ingredients LIMIT 20';

    try {
        const result = await session.run(query);
        const recipes = result.records.map(record => ({
            name: record.get('name'),
            time: record.get('time'),
            difficulty: record.get('difficulty'),
            ingredients: record.get('ingredients') || []
        }));

        res.json(recipes);
    } catch (error) {
        console.error("Error fetching recipes:", error);
        res.status(500).send('Internal Server Error');
    } finally {
        await session.close();
    }
});
  
app.post('/recipes', async (req, res) => {
    const { name, time, difficulty, ingredients } = req.body;
    const session = driver.session();

    try {
        const checkQuery = 'MATCH (r:Recipe {name: $name}) RETURN r';
        const checkResult = await session.run(checkQuery, { name });

        if (checkResult.records.length > 0) {
            return res.status(400).json({ message: 'Przepis o tej nazwie już istnieje' });
        }

        const createQuery = `
            CREATE (r:Recipe {name: $name, time: $time, difficulty: $difficulty})
            WITH r
            UNWIND $ingredients AS ingredient
            MERGE (i:Ingredient {name: ingredient})
            MERGE (r)-[:USES]->(i)
            RETURN r
        `;
        const createResult = await session.run(createQuery, { 
            name, 
            time, 
            difficulty, 
            ingredients 
        });

        res.status(201).json({ 
            message: 'Przepis został dodany pomyślnie', 
            recipe: createResult.records[0].get('r').properties 
        });
    } catch (error) {
        console.error("Error creating recipe:", error);
        res.status(500).send('Internal Server Error');
    } finally {
        await session.close();
    }
});

app.get('/ingredients', async (req, res) => {
    const session = driver.session();

    try {
        const result = await session.run('MATCH (i:Ingredient) RETURN i.name AS name');
        const ingredients = result.records.map(record => record.get('name'));

        res.json(ingredients);
    } catch (error) {
        console.error("Error fetching ingredients:", error);
        res.status(500).send('Internal Server Error');
    } finally {
        await session.close();
    }
});

app.post('/ingredients', async (req, res) => {
    const { name } = req.body;
    const session = driver.session();

    try {
        const query = `
            MERGE (i:Ingredient {name: $name})
            RETURN i
        `;
        const result = await session.run(query, { name });

        res.status(201).json({ message: 'Ingredient created successfully', ingredient: result.records[0].get('i').properties });
    } catch (error) {
        console.error("Error creating ingredient:", error);
        res.status(500).send('Internal Server Error');
    } finally {
        await session.close();
    }
});
  
app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});