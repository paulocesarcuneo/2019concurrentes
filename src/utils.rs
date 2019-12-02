use std::collections::HashMap;

pub trait GroupBy<A> {
    fn group_by<F, K>(&self, f: F) -> HashMap<K, Vec<&A>>
    where
        F: Fn(&A) -> K,
        K: std::hash::Hash + std::cmp::Eq + Copy;
}

impl<A> GroupBy<A> for Vec<A> {
    fn group_by<F, K>(&self, f: F) -> HashMap<K, Vec<&A>>
    where
        F: Fn(&A) -> K,
        K: std::hash::Hash + std::cmp::Eq + Copy,
    {
        let mut m = HashMap::new();
        for i in self {
            let k = f(&i);
            if !m.contains_key(&k) {
                m.insert(k, Vec::new());
            }
            m.get_mut(&k).unwrap().push(i);
        }
        m
    }
}
